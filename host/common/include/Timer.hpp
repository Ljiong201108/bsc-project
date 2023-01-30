#pragma once
#include <chrono>

class Timer{
public:
	static std::chrono::nanoseconds totalTime, computeTime, fpgaIOTime, hostIOTime, fpgaInitTime, anaTime;
	static std::chrono::_V2::steady_clock::time_point totalStart, computeStart, fpgaIOStart, hostIOStart, fpgaInitStart, anaStart;

	static void startTotalTimer();

	static void startComputeTimer();

	static void endTotalTimer();

	static void endComputeTimer();

	static void startFPGAIOTimer();

	static void endFPGAIOTimer();

	static void startHostIOTimer();

	static void endHostIOTimer();

	static void startFPGAInitTimer();

	static void endFPGAInitTimer();

	static void startAnaTimer();

	static void endAnaTimer();

	static void reset();
};

