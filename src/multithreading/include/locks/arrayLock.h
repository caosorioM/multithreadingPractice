#include <atomic>
#include <stdlib.h>

template <size_t MAX_THREADS>
class arrayLock
{
public:

    arrayLock()
    {

    }

    void lock()
    {
        _slot = _current.fetch_add(1 , std::memory_order_relaxed)%MAX_THREADS;
        while(!_array[_slot].val.load(std::memory_order_acquire)){}
    }

    void unlock()
    {
        _array[_slot].val.store(false, std::memory_order_relaxed);
        _array[(_slot + 1)%MAX_THREADS].val.store(true , std::memory_order_release);
    }


private:

    struct flag
    {
        alignas(std::hardware_destructive_interference_size) std::atomic_bool val;
    };
    flag                       _array[MAX_THREADS];
    static thread_local size_t _slot;
    std::atomic<size_t>        _current; 
};