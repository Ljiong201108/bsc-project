#include "Timer.hpp"
#include <iostream>

std::chrono::nanoseconds Timer::totalTime;
std::chrono::nanoseconds Timer::computeTime;
std::chrono::nanoseconds Timer::fpgaIOTime;
std::chrono::nanoseconds Timer::hostIOTime;
std::chrono::nanoseconds Timer::fpgaInitTime;
std::chrono::nanoseconds Timer::anaTime;
std::chrono::_V2::steady_clock::time_point Timer::totalStart;
std::chrono::_V2::steady_clock::time_point Timer::computeStart;
std::chrono::_V2::steady_clock::time_point Timer::fpgaIOStart;
std::chrono::_V2::steady_clock::time_point Timer::hostIOStart;
std::chrono::_V2::steady_clock::time_point Timer::fpgaInitStart;
std::chrono::_V2::steady_clock::time_point Timer::anaStart;

void Timer::startTotalTimer(){
	totalStart=std::chrono::steady_clock::now();
}

void Timer::startComputeTimer(){
	computeStart=std::chrono::steady_clock::now();
}

void Timer::startFPGAIOTimer(){
	fpgaIOStart=std::chrono::steady_clock::now();
}

void Timer::startHostIOTimer(){
	hostIOStart=std::chrono::steady_clock::now();
}

void Timer::startFPGAInitTimer(){
	fpgaInitStart=std::chrono::steady_clock::now();
}

void Timer::startAnaTimer(){
	anaStart=std::chrono::steady_clock::now();
}

void Timer::endTotalTimer(){
	auto now=std::chrono::steady_clock::now();
	totalTime+=std::chrono::nanoseconds{now-totalStart};
	// std::cout<<"total time += "<<(now-totalStart).count()<<std::endl;
}

void Timer::endComputeTimer(){
	auto now=std::chrono::steady_clock::now();
	computeTime+=std::chrono::nanoseconds{now-computeStart};
	// std::cout<<"compute time += "<<(now-computeStart).count()<<std::endl;
}

void Timer::endFPGAIOTimer(){
	auto now=std::chrono::steady_clock::now();
	fpgaIOTime+=std::chrono::nanoseconds{now-fpgaIOStart};
	// std::cout<<"fpga io time += "<<(now-fpgaIOStart).count()<<std::endl;
}

void Timer::endHostIOTimer(){
	auto now=std::chrono::steady_clock::now();
	hostIOTime+=std::chrono::nanoseconds{now-hostIOStart};
	// std::cout<<"host io time += "<<(now-hostIOStart).count()<<std::endl;
}

void Timer::endFPGAInitTimer(){
	auto now=std::chrono::steady_clock::now();
	fpgaInitTime+=std::chrono::nanoseconds{now-fpgaInitStart};
	// std::cout<<"fpga io time += "<<(now-fpgaIOStart).count()<<std::endl;
}

void Timer::endAnaTimer(){
	auto now=std::chrono::steady_clock::now();
	anaTime+=std::chrono::nanoseconds{now-anaStart};
	// std::cout<<"host io time += "<<(now-hostIOStart).count()<<std::endl;
}

void Timer::reset(){
	totalTime=std::chrono::nanoseconds{0};
	computeTime=std::chrono::nanoseconds{0};
	fpgaIOTime=std::chrono::nanoseconds{0};
	hostIOTime=std::chrono::nanoseconds{0};
	fpgaInitTime=std::chrono::nanoseconds{0};
	anaTime=std::chrono::nanoseconds{0};
}