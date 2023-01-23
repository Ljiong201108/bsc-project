#include <chrono>

class Timer{
public:
	static std::chrono::nanoseconds totalTime, computeTime;
	static std::chrono::_V2::steady_clock::time_point totalStart, computeStart;

	static void startTotalTimer();

	static void startComputeTimer();

	static void endTotalTimer();

	static void endComputeTimer();

	static void reset();
};

