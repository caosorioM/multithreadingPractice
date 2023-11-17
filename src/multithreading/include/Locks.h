#pragma once
#include<atomic>

class spinlock
{
private:
    std::atomic_flag _flag;
public:

    void lock()
    {
        while(_flag.test_and_set()){}
    }

    void unlock()
    {
        _flag.clear();
    }
};