#include<chrono>

class backOff
{
public:

   backOff(std::chrono::duration<double, std::micro> _maxDuration)
   {
        _start = std::chrono::system_clock::now();
   };

private:
    std::chrono::duration<double, std::micro> _max;
    std::chrono::system_clock::time_point     _start;
};