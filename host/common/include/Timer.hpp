#include <chrono>

class Timer{
public:
	static std::chrono::nanoseconds totalTime, computeTime, fpgaIOTime, hostIOTime;
	static std::chrono::_V2::steady_clock::time_point totalStart, computeStart, fpgaIOStart, hostIOStart;

	static void startTotalTimer();

	static void startComputeTimer();

	static void endTotalTimer();

	static void endComputeTimer();

	static void startFPGAIOTimer();

	static void endFPGAIOTimer();

	static void startHostIOTimer();

	static void endHostIOTimer();

	static void reset();
};

