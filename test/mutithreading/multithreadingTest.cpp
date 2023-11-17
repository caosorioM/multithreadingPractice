#include <latch>
#include <thread>
#include <vector>
#include "Locks.h"
#include <mutex>
#include "gtest/gtest.h"

TEST(locks , spinlock)
{
    const auto numberOfThreads = std::thread::hardware_concurrency();
    std::latch barrier{numberOfThreads};
    std::vector<std::thread> threads;

    spinlock  testMutex;
    size_t    count = 0;

    for(size_t threadIndex = 0; threadIndex < numberOfThreads ; ++threadIndex)
    {
        threads.emplace_back([&barrier , &testMutex , &count]()
        {
            barrier.count_down();
            std::lock_guard<spinlock>lk(testMutex);

            ++count;
        });
    }

    EXPECT_EQ(count , numberOfThreads );
}