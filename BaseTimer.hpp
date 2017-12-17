#ifndef _BASE_TIMER_HPP_
#define _BASE_TIMER_HPP_

// AZT
class BaseTimer {

    public:
        virtual void onAction() = 0;
        virtual ~BaseTimer() {};
        
};

#endif