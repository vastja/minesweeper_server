#ifndef _ROUND_TIMER_HPP_
#define _ROUND_TIMER_HPP_

#include <iostream>
#include <condition_variable>
#include <thread>
#include <chrono>

#include "BaseTimer.hpp"

class Game;

class Timer {

    private:

        std::condition_variable cv;
        std::mutex m;

    public:

        void start(const int time,  BaseTimer * bt);
        void cancel();

};

#endif