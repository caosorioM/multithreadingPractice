#include <benchmark/benchmark.h>
#include <vector>
#include "Locks.h"
#include <mutex>
#include <limits>

spinlock mutex;
size_t   count = 0;
constexpr size_t loopSize = 10;
constexpr size_t   goal =  (1ull)<<32;

static void BM_spinlockTest(benchmark::State& state)
{
    if(state.thread_index() == 0)
    {
        count = 0;
    }

    for(auto _ : state)
    {
        while(true)
        {
            std::lock_guard<spinlock> lk(mutex);

            if(count >= goal)
                break;

            for(size_t i = 0;  i< loopSize ; ++i)
            {
               benchmark::DoNotOptimize(++count);
            }
        }
    }
    
}

BENCHMARK(BM_spinlockTest)->ThreadRange(1,10)->UseRealTime();
BENCHMARK_MAIN();