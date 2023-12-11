#pragma once
#include <atomic>
#include "util.h"
#include <bitset>
#include <array>
#include "nodeArrays.h"

class spinlock
{
public:

    spinlock(){_flag.clear(std::memory_order_relaxed);}

    void lock()
    {
        while(_flag.test_and_set(std::memory_order_acquire)){}
    }

    void unlock()
    {
        _flag.clear(std::memory_order_release);
    }
private:
    std::atomic_flag _flag;
};

class betterSpinLock
{
public:
    betterSpinLock(){_flag.store(true, std::memory_order_relaxed);}

    void lock()
    {
        while(true)
        {
            if(_flag.load(std::memory_order_relaxed))
            {
                auto val = true;
                if(_flag.compare_exchange_strong(val, false , std::memory_order_acquire, std::memory_order_relaxed))
                    break;
            }
        }
    }

    void unlock()
    {
        _flag.store(true, std::memory_order_release);
    }
private:
    std::atomic_bool _flag;
};

class backoffLock
{
public:
    void lock()
    {
        backOff<std::chrono::microseconds , 1, 100 > theBackOff;

        while(true)
        {
            if(_flag.load(std::memory_order_relaxed))
            {
                auto val = false;
                if(_flag.compare_exchange_strong(val, std::memory_order_acquire, std::memory_order_relaxed))
                {
                    break;
                }
            }
            theBackOff.sleep();
        }
    }

    void unlock()
    {
        _flag.store(true, std::memory_order_release);
    }

private:
    std::atomic_bool _flag;
};

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

class CLHLock
{
public:

void lock()
{
    _currentNode->_locked.store(true, std::memory_order_relaxed);
    _predecesor = _tail.exchange(_currentNode, std::memory_order_release);

    while(_predecesor->_locked.load(std::memory_order_acquire)){}
}

void unlock()
{
    _currentNode->_locked.store(false, std::memory_order_release);
    _currentNode = _predecesor;
}

struct node
{
    node*              _predecesor;
    std::atomic<bool>  _locked;
};
private:
std::atomic<node*>  _tail;
static thread_local node*  _currentNode;
static thread_local node*  _predecesor;
};


class MCSLock
{
public:
    void lock()
    {
        auto predecesor = _tail.exchange(&_myNode , std::memory_order_relaxed);

        if(predecesor)
        {
            _myNode.locked.store(true , std::memory_order_relaxed);
            predecesor->succesor.store(&_myNode , std::memory_order_release);

            while(!_myNode.locked.load(std::memory_order_acquire)){}
        }
    }

    void unlock()
    {
        auto succesor = _myNode.succesor.load(std::memory_order_relaxed);

        if(!succesor)
        {
            auto pMyNode = &_myNode;
            if(_tail.compare_exchange_strong( pMyNode , nullptr ,std::memory_order_release , std::memory_order_relaxed))
            {
                return;
            }

            while(succesor =_myNode.succesor.load(std::memory_order_acquire)){}
        }

        succesor->locked.store(false, std::memory_order_release);
    }

private:
struct node
{
    std::atomic<node*> succesor;
    std::atomic<bool>  locked;
};

std::atomic<node*>       _tail;
static thread_local node _myNode;
};


class ToLock
{
public:
    void lock()
    {
        auto predecesor = _tail.exchange(&_myNode);

        if(predecesor == nullptr || predecesor->predecesor.load() == &AVAILABLE)
            return;

        timer t; 

        while(t.getTimeDiff<std::chrono::microseconds>() < _patience)
        {
            auto predecesorPredecesor = predecesor->predecesor.load();

            if(predecesorPredecesor == &AVAILABLE)
                return;

            if(predecesorPredecesor)
            {
                _myNode.predecesor.store(predecesorPredecesor);
            }
        }

        auto pMyNode = &_myNode;
        if(!_tail.compare_exchange_strong(pMyNode , nullptr))
        {
            pMyNode->predecesor.store(predecesor);
        }
    }

    void unlock()
    {
        auto pMyNode = &_myNode;
        if(!_tail.compare_exchange_strong(pMyNode ,  nullptr))
            _myNode.predecesor.store(&AVAILABLE);
    }

private:
    struct node
    {
        std::atomic<node*> predecesor;
    };
    static thread_local node _myNode;
    static node AVAILABLE;

    std::atomic<node*> _tail;

    std::chrono::microseconds _patience;
};

class compositeLock
{
public:
    bool trylock()
    {
        timer t;
        node* pNode =  ClaimNode( t);

        if(!pNode)
            return false;

        if(!QueueNode(t , pNode) )
            return false;

        if(!WaitInQueue(t, pNode))
            return false;

        return true;
    }

    void unlock()
    {
        _myNode->_state.store(RELEASED, std::memory_order_release);
        _myNode = nullptr;
    }

private:
    enum state {FREE , WAITING , RELEASED , ABORTED};
    struct node
    {
        std::atomic<state> _state;
        std::atomic<node*> _predecesor;
    };


    node* ClaimNode( timer& t)
    {
        auto nodeIndex = GenerateNumber(_dist);
        auto currentNode = _nodeArray.GetNode(nodeIndex);
        state currentState = FREE;

        if(currentNode->_state.compare_exchange_strong(currentState , WAITING , 
                                                        std::memory_order_relaxed ,std::memory_order_relaxed))
        {
            return currentNode;
        }

        backOff<std::chrono::microseconds , 1, 100 > theBackOff;

        while(true)
        {
            auto currentTail = _tail.load(std::memory_order_relaxed);
       
            if(currentState == ABORTED || currentState == RELEASED)
            {
                if(currentNode == _nodeArray.GetNode(currentTail) )
                {
                    node* myPred = nullptr;

                    if(currentNode->_state.load(std::memory_order_relaxed) == ABORTED)
                    {
                        myPred = currentNode->_predecesor.load(std::memory_order_relaxed);
                    }

                    if(_tail.compare_exchange_strong(currentTail , CreateNextTag(currentTail , myPred) 
                                                                 , std::memory_order_relaxed ,  std::memory_order_relaxed ))
                    {
                        currentNode->_state.store(WAITING , std::memory_order_relaxed);
                        return currentNode;
                    }
                }
            }
        }

        theBackOff.sleep();

        if(t.getTimeDiff<std::chrono::microseconds>() > _patience)
            return nullptr;
    }

    bool QueueNode(timer& t , node* pNode)
    {
        auto currentTail = _tail.load(std::memory_order_relaxed);
        nodeTag nextTail;

        do
        {
            if(t.getTimeDiff<std::chrono::microseconds>() > _patience)
            {
                pNode->_state.store(FREE , std::memory_order_relaxed);
                return false;
            }

            nextTail = CreateNextTag( currentTail , pNode);
        }
        while(_tail.compare_exchange_strong(currentTail , nextTail , std::memory_order_release, std::memory_order_relaxed));

        pNode->_predecesor.store(_nodeArray.GetNode(currentTail));
        return true;
    }

    bool WaitInQueue(timer& t, node* pNode)
    {
       auto predecesor = _myNode->_predecesor.load(std::memory_order_acquire);

        if( !predecesor )
        {
            _myNode = pNode;
            return true;
        }

       auto predcesorState = predecesor->_state.load(std::memory_order_acquire);

       while(predcesorState != RELEASED)
       {
          if(predcesorState == ABORTED)
          {
            auto oldPredecesor = predecesor;
            predecesor = predecesor->_predecesor.load(std::memory_order_relaxed);
            predecesor->_state.store(FREE, std::memory_order_release);
          }

          if(t.getTimeDiff<std::chrono::microseconds>() > _patience)
          {
            pNode->_state.store(ABORTED, std::memory_order_relaxed);
            return false;
          }

          predcesorState = predecesor->_state.load(std::memory_order_acquire);
       }

        _myNode = pNode;
        predecesor->_state.store(FREE , std::memory_order_relaxed);
    }

    std::atomic<nodeTag> _tail;

    std::chrono::duration<std::micro> _patience;

    node* _myNode;
    stampedNodesArray<node , 10> _nodeArray;
    std::uniform_int_distribution<uint32_t> _dist;
};


class fastPathLock
{
public:

    bool tryLock()
    {
        if(fastPath())
            return true;

        if(_lock.trylock() )
        {
            while(IsFastPathEnabled(_tail.load())){}

            return true;
        }

        return false;
    }

    void unlock()
    {
        if(!unlockFastPath())
        {
            _lock.unlock();
        }
    }

private:
    bool fastPath()
    {
        auto currentTail = _tail.load();

        if(!IsNull(currentTail))
            return false;

        if(!IsFastPathEnabled(currentTail))
            return false;


        return _tail.compare_exchange_strong(currentTail , EnableFastPath(currentTail));

    }

    bool unlockFastPath()
    {
        auto currentTail = _tail.load();

        if(!IsFastPathEnabled(currentTail) )
            return currentTail;

        nodeTag newTail;
        do
        {
           newTail = DisableFastPath(currentTail);
        } while (!_tail.compare_exchange_strong(currentTail , newTail) );
        
        return true;
    }

    bool IsFastPathLocked(nodeTag tag);
    nodeTag enableFastPath(nodeTag tag);

    compositeLock _lock;
    std::atomic<nodeTag> _tail;
};