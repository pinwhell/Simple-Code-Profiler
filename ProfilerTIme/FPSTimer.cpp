#include "FPSTimer.h"
#include <thread>

void FPSTimer::WaitNextFrame()
{
	m_NextFrameStart = m_CurrFrameEpoch + std::chrono::milliseconds((int)m_MillisBetweenFrames);

	std::this_thread::sleep_until(m_NextFrameStart);
}

void FPSTimer::PreFrame()
{
	m_CurrFrameEpoch = std::chrono::steady_clock::now();
	m_DeltaTime = (double)std::chrono::duration_cast<std::chrono::milliseconds>(m_CurrFrameEpoch - m_LastFrameEpoch).count();
}

void FPSTimer::PostFrame()
{
	m_DeltaTimeAddend += getDeltaTimeMillis();

	if (m_DeltaTimeAddend >= 1000)
	{
		m_CachedFrameRate = m_FramesCount;
		m_FramesCount = 0;

		while (m_DeltaTimeAddend >= 1000)
		{
			m_DeltaTimeAddend -= 1000;
		}
	}

	m_FramesCount++;
	m_LastFrameEpoch = m_CurrFrameEpoch;
}

float FPSTimer::getDeltaTime()
{
	return (float)m_DeltaTime / (float)1000.f;
}

float FPSTimer::getDeltaTimeMillis()
{
	return (float)m_DeltaTime;
}

int FPSTimer::getCachedFPS()
{
	return m_CachedFrameRate;
}

int FPSTimer::getTargetFPS()
{
	return m_CapedFPS;
}

void FPSTimer::setTargetFPS(int targetFPS)
{
	m_CapedFPS = targetFPS;
	m_MillisBetweenFrames = double(1000 / (m_CapedFPS + 4));
}
