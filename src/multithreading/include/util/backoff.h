#pragma once
#include <random>

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