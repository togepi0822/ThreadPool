#ifndef INCLUDE_TP_THREAD_H
#define INCLUDE_TP_THREAD_H

#include <tp/config.h>

#include <functional>
#include <future>

NAMESPACE_TP_BEGIN

class Thread {
    using ThreadFunc = std::function<void(int)>;
public:

    explicit Thread(ThreadFunc func)
        : func_(std::move(func)), threadId_(generatedId) {
        ++generatedId;
    }

    ~Thread() = default;

    void start(const int id) {
        std::thread t(func_, id);
        t.detach();
    }

    [[nodiscard]] int getId() const {
        return threadId_;
    }

private:
    static int generatedId;
    ThreadFunc func_;
    int threadId_;
};

int Thread::generatedId = 0;

NAMESPACE_TP_END

#endif //INCLUDE_TP_THREAD_H
