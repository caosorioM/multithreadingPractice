#pragma once
#include<chrono>
#include<random>
#include<thread>

class rng
{
public:
    thread_local static  std::default_random_engine  _engine;
};

template< typename distribution>
auto  GenerateNumber(const distribution& dist)
{
    thread_local static  std::default_random_engine  engine(1639);
    return dist(engine);

}

template< class unit, unit MIN , unit MAX >
class backOff
{
public:

   backOff():_limit(max)
   {
   };

   std::chrono::duration<double, std::micro>   GetBackoff()
   {
        
   }

private:
    unit _limit;
};