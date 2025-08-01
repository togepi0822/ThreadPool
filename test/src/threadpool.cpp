#include <gtest/gtest.h>

#include <tp/threadpool.h>

#include <future>
#include <cassert>
#include <chrono>
#include <thread>
#include <string>

using namespace tp;

using uLong = unsigned long;

uLong sum(const int a, const int b) {
    assert(a <= b);

    uLong res = 0;
    for (int i = a; i <= b; ++i) {
        res += i;
    }
    return res;
}

uLong sum_delay(const int a, const int b) {
    assert(a <= b);

    std::this_thread::sleep_for(std::chrono::seconds(2));
    uLong res = 0;
    for (int i = a; i <= b; ++i) {
        res += i;
    }
    return res;
}

std::string add(const std::string& a, const std::string& b) {
    return a + b;
}

TEST(threadpool, CACHED) {
    ThreadPool pool(PoolMode::CACHED, 2);

    std::future<uLong> res1 = pool.submitTask(sum, 1, 100'000'000);
    std::future<uLong> res2 = pool.submitTask(sum, 100'000'001, 200'000'000);
    pool.submitTask(sum, 2001, 3000);
    pool.submitTask(sum, 3001, 4000);
    pool.submitTask(sum, 4001, 5000);

    ASSERT_EQ(3849838848, res1.get() + res2.get());
}

TEST(threadpool, FIXED) {
    ThreadPool pool(PoolMode::FIXED, 2);

    std::future<uLong> res1 = pool.submitTask(sum, 1, 100'000'000);
    std::future<uLong> res2 = pool.submitTask(sum, 100'000'001, 200'000'000);
    pool.submitTask(sum, 2001, 3000);
    pool.submitTask(sum, 3001, 4000);
    pool.submitTask(sum, 4001, 5000);

    ASSERT_EQ(3849838848, res1.get() + res2.get());
}

TEST(threadpool, SubmissionTimeOut) {
    ThreadPool pool(PoolMode::FIXED, 2, 2);

    std::future<uLong> res1 = pool.submitTask(sum_delay, 1, 100'000'000);
    std::future<uLong> res2 = pool.submitTask(sum_delay, 100'000'001, 200'000'000);
    pool.submitTask(sum_delay, 2001, 3000);
    pool.submitTask(sum_delay, 3001, 4000);
    std::future<uLong> res3 = pool.submitTask(sum_delay, 4001, 5000);

    ASSERT_EQ(3849838848, res1.get() + res2.get());
    ASSERT_EQ(0, res3.get());
}

TEST(threadpool, SubmissionWithLvalueAndRvalue) {
    ThreadPool pool(PoolMode::FIXED, 2);

    std::string s{"hello "};
    std::future<std::string> res = pool.submitTask(add, s, "world");

    ASSERT_EQ(std::string{"hello world"}, res.get());
}