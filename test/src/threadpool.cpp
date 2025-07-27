#include <gtest/gtest.h>

#include <tp/threadpool.h>

#include <future>
#include <cassert>

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

TEST(threadpool, CACHED) {
    ThreadPool pool;
    pool.setMode(PoolMode::CACHED);
    pool.start(2);

    std::future<uLong> res1 = pool.submitTask(sum, 1, 100'000'000);
    std::future<uLong> res2 = pool.submitTask(sum, 100'000'001, 200'000'000);
    pool.submitTask(sum, 2001, 3000);
    pool.submitTask(sum, 3001, 4000);
    pool.submitTask(sum, 4001, 5000);

    ASSERT_EQ(3849838848, res1.get() + res2.get());
}

TEST(threadpool, FIXED) {
    ThreadPool pool;
    pool.setMode(PoolMode::FIXED);
    pool.start(2);

    std::future<uLong> res1 = pool.submitTask(sum, 1, 100'000'000);
    std::future<uLong> res2 = pool.submitTask(sum, 100'000'001, 200'000'000);
    pool.submitTask(sum, 2001, 3000);
    pool.submitTask(sum, 3001, 4000);
    pool.submitTask(sum, 4001, 5000);

    ASSERT_EQ(3849838848, res1.get() + res2.get());
}