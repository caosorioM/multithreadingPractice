#pragma once
#include <atomic>
#include <random>
#include "../util/timer.h"
#include "../util/rng.h"
#include "../util/backoff.h"
#include "../util/nodeArrays.h"


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
