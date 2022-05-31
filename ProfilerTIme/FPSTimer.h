#pragma once
#include <chrono>

#define SEC_TO_MILLIS(sec) (sec) * 1000
#define MILLIS_TO_SEC(mill) (mill) / 1000
#define TRANSCURRED_TIME(t)  FPSTimer::GetCurrMillis() - (t)

class FPSTimer
{
private:
	int m_CachedFrameRate;
	int m_FramesCount;
	int m_CapedFPS;
	double m_MillisBetweenFrames;
	double m_DeltaTimeAddend;
	double m_DeltaTime;
	std::chrono::steady_clock::time_point m_LastFrameEpoch;
	std::chrono::steady_clock::time_point m_CurrFrameEpoch;
	std::chrono::steady_clock::time_point m_NextFrameStart;

public:

	FPSTimer(uintptr_t fps)
		: m_CachedFrameRate(0)
		, m_CapedFPS(fps)
		, m_DeltaTimeAddend(0.0)
		, m_FramesCount(0)
		, m_LastFrameEpoch(std::chrono::steady_clock::now())
		, m_CurrFrameEpoch(std::chrono::steady_clock::now())
		, m_NextFrameStart(std::chrono::steady_clock::now())
		, m_DeltaTime(0.0)
	{
		setTargetFPS(fps);
	}

	void PreFrame();
	void PostFrame();
	void WaitNextFrame();
	float getDeltaTime();
	float getDeltaTimeMillis();
	int getCachedFPS();
	int getTargetFPS();
	void setTargetFPS(int targetFPs);
	static uint32_t GetCurrMillis() {
		return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count());
	}
};

