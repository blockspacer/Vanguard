#include <cmath>
#include <iostream>

#include "timer.h"

void Timer::init(double duration_, std::function<void(Gamestate &state)> eventfunc_, bool active_)
{
    Timer::init(duration_, active_);
    eventfunc = eventfunc_;
}

void Timer::init(double duration_, bool active_)
{
    timer = 0;
    active = active_;
    eventfunc = nullptr;
    duration = duration_;
    inited = true;
}

void Timer::reset()
{
    timer = 0.0;
    active=true;
}

void Timer::update(Gamestate &state, double dt)
{
    if (not inited)
    {
        std::cout << "Fatal error: Timer update was called before init function!";
        throw -1;
    }

    if (active)
    {
        timer += dt;
        if (timer >= duration)
        {
            if (eventfunc != 0)
            {
                eventfunc(state);
            }
            if (reset_after_eventfunc_flag)
            {
                reset();
                reset_after_eventfunc_flag = false;
            }
            else
            {
                active = false;
            }
        }
    }
}

double Timer::getpercent()
{
    if (not inited)
    {
        std::cout << "Fatal error: Timer getpercent was called before init function!";
        throw -1;
    }

    // Max and min are for rounding errors
    return std::fmax(std::fmin(timer/duration, 1.0), 0.0);
}

void Timer::interpolate(Timer &prev_timer, Timer &next_timer, double alpha)
{
    if (not inited)
    {
        std::cout << "Fatal error: Timer interpolate was called before init function!";
        throw -1;
    }

    Timer &preferredtimer = alpha < 0.5 ? prev_timer : next_timer;
    active = preferredtimer.active;
    if (prev_timer.active and next_timer.active)
    {
        timer = prev_timer.timer + alpha*(next_timer.timer - prev_timer.timer);
    }
    else
    {
        timer = preferredtimer.timer;
    }
}

void Timer::reset_after_eventfunc()
{
    reset_after_eventfunc_flag = true;
}
