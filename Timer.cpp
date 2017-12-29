
#include <iostream>
#include <condition_variable>
#include <thread>
#include <chrono>

#include "Timer.hpp"
#include "Game.hpp"

void Timer::start(const int time, BaseTimer * bt) {

    std::unique_lock<std::mutex> lk(m);
    if (cv.wait_for(lk, std::chrono::seconds(time)) == std::cv_status::timeout) {
        //std::cerr << "TIME OUT\n";
        if (bt != NULL) {
            bt->onAction();
        }
        
    } 
    else {
        //std::cerr << "INTERRUPTED\n";
    }

}

void Timer::cancel() {
    cv.notify_all();
}