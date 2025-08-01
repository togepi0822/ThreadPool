#ifndef INCLUDE_TP_THREADPOOL_H
#define INCLUDE_TP_THREADPOOL_H

#include <tp/thread.h>
#include <tp/config.h>
#include <tp/log.h>

#include <atomic>
#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>
#include <unordered_map>

NAMESPACE_TP_BEGIN

enum class PoolMode {
    FIXED,
    CACHED
};

class ThreadPool {
    static constexpr int TASK_MAX_NUM = INT32_MAX;
    static constexpr int THREAD_MAX_NUM = 100;
    static constexpr int THREAD_MAX_IDLE_TIME = 10;

    using Task = std::function<void()>;

public:
    explicit ThreadPool(const PoolMode poolMode = PoolMode::FIXED,
        const size_t initThreadNum = std::thread::hardware_concurrency(),
        const size_t threadMaxNum = THREAD_MAX_NUM,
        const size_t taskMaxNum = TASK_MAX_NUM)
        : threadMaxNum_(threadMaxNum),
          initThreadNum_(initThreadNum),
          taskMaxNum_(taskMaxNum),
          poolMode_(poolMode) {
        init();
    }

    ~ThreadPool() {
        isWorking_ = false;

        std::unique_lock<std::mutex> lock(mtx_);
        notEmpty_.notify_all();

        exit_.wait(lock, [&]() -> bool { return threads_.empty(); });
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

public:
    template<class F, class... Args>
    auto submitTask(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        using Ret = decltype(f(args...));
        using std::chrono::seconds;

        auto task = std::make_shared<std::packaged_task<Ret()>>(
            [f = std::forward<F>(f), ...args = std::forward<Args>(args)]() mutable -> Ret {
                return f(args...);
            }
        );

        std::unique_lock<std::mutex> lock(mtx_);

        if (!notFull_.wait_for(lock, seconds(1), [&]() -> bool { return taskQue_.size() < taskMaxNum_; })) {
            LOGERROR("task queue is full, submission faild\n");
            auto taskDropped = std::make_shared<std::packaged_task<Ret()>>(
                []() -> Ret { return Ret(); }
            );
            (*taskDropped)();
            return taskDropped->get_future();
        }

        taskQue_.emplace([=] { (*task)(); });
        notEmpty_.notify_all();

        if (poolMode_ == PoolMode::CACHED && taskQue_.size() > idleThreadNum_
            && threads_.size() < threadMaxNum_) {
            auto ptr = std::make_unique<Thread>([this](const int id) { this->threadFunc(id); });
            int threadId = ptr->getId();
            LOGINFO("extra thread created\n");

            threads_.emplace(threadId, std::move(ptr));
            threads_[threadId]->start(threadId);
            ++idleThreadNum_;
        }

        return task->get_future();
    }



private:
    void init() {
        for (size_t i = 0; i < initThreadNum_; ++i) {
            auto ptr = std::make_unique<Thread>([this](const int id) { this->threadFunc(id); });
            int threadId = ptr->getId();
            threads_.emplace(threadId, std::move(ptr));
        }

        for (size_t i = 0; i < initThreadNum_; ++i) {
            threads_[static_cast<int>(i)]->start(static_cast<int>(i));
            ++idleThreadNum_;
        }

        isWorking_ = true;
    }

    void threadFunc(const int threadId) {
        using std::chrono::seconds;
        using std::chrono::high_resolution_clock;
        using std::chrono::duration_cast;
        using std::cv_status::timeout;

        auto lastTime = high_resolution_clock::now();

        while (true) {
            Task task;
            {
                std::unique_lock<std::mutex> lock(mtx_);

                while (taskQue_.empty()) {
                    if (!isWorking_) {
                        threads_.erase(threadId);
                        exit_.notify_all();
                        return;
                    }

                    if (poolMode_ == PoolMode::CACHED) {
                        if (timeout == notEmpty_.wait_for(lock, seconds(1))) {
                            auto curTime = high_resolution_clock::now();
                            auto idleTime = duration_cast<seconds>(curTime - lastTime);
                            if (idleTime.count() > THREAD_MAX_IDLE_TIME
                                && threads_.size() > initThreadNum_) {
                                threads_.erase(threadId);
                                --idleThreadNum_;
                                exit_.notify_all();
                                return;
                            }
                        }
                    } else {
                        notEmpty_.wait(lock);
                    }
                } // while (taskQue_.size() == 0)

                --idleThreadNum_;
                task = std::move(taskQue_.front());
                taskQue_.pop();

                if (!taskQue_.empty()) {
                    notEmpty_.notify_all();
                }
                notFull_.notify_all();
            }

            if (task != nullptr) {
                task();
            }
            ++idleThreadNum_;
            lastTime = high_resolution_clock::now();
        }
    }

private:
    // 线程相关
    std::unordered_map<int, std::unique_ptr<Thread>> threads_;
    size_t threadMaxNum_;
    size_t initThreadNum_ = 0;
    std::atomic<size_t> idleThreadNum_ = 0;

    // 任务队列相关
    std::queue<Task> taskQue_;
    size_t taskMaxNum_;

    // 线程同步相关
    std::mutex mtx_;
    std::condition_variable notFull_;
    std::condition_variable notEmpty_;
    std::condition_variable exit_;

    // 线程池工作状态相关
    PoolMode poolMode_;
    std::atomic<bool> isWorking_ = false;
};

NAMESPACE_TP_END

#endif //INCLUDE_TP_THREADPOOL_H