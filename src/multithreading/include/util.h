#pragma once
#include<chrono>
#include<random>
#include<thread>


template< typename distribution>
auto  GenerateNumber(distribution& dist)
{
    thread_local static  std::random_device rd;
    thread_local static  std::default_random_engine  engine(rd());

    return dist(engine);

}

template< class unit, int MIN , int MAX >
class backOff
{
public:

   backOff():_limit(MIN)
   {
   };

   void  sleep()
   {
        int delay =  GenerateNumber(_dist)%_limit; 
        _limit = std::min(2*_limit , MAX);

        std::this_thread::sleep_for( unit(delay) );
   }

private:
    int _limit;
    std::uniform_int_distribution<> _dist = std::uniform_int_distribution<>(MIN , MAX);
};