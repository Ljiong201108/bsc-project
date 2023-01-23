#include "Timer.hpp"

std::chrono::nanoseconds Timer::totalTime;
std::chrono::nanoseconds Timer::computeTime;
std::chrono::_V2::steady_clock::time_point Timer::totalStart;
std::chrono::_V2::steady_clock::time_point Timer::computeStart;

void Timer::startTotalTimer(){
	totalStart=std::chrono::steady_clock::now();
}

void Timer::startComputeTimer(){
	computeStart=std::chrono::steady_clock::now();
}

void Timer::endTotalTimer(){
	totalTime+=std::chrono::nanoseconds{std::chrono::steady_clock::now()-totalStart};
}

void Timer::endComputeTimer(){
	computeTime+=std::chrono::nanoseconds{std::chrono::steady_clock::now()-computeStart};
}

void Timer::reset(){
	totalTime=std::chrono::nanoseconds{0};
	computeTime=std::chrono::nanoseconds{0};
}