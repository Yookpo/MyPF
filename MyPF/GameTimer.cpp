#include "GameTimer.h"

namespace My
{
	GameTimer::GameTimer()
		: m_deltaTime{ 0.0f }, m_totalTime{ 0.0f}
	{
		Reset();
	}

	void GameTimer::Reset()
	{
		auto curTime = steady_clock::now();

		m_prevTime = m_startTime = curTime;
		m_deltaTime = m_totalTime = 0.0f;
	}
	void GameTimer::Tick()
	{
		auto curTime = steady_clock::now();

		// 현재 - 직전을 초 단위로 변환
		m_deltaTime = duration<float>(curTime - m_prevTime).count();

		// 현재 - 시작 시간을 초 단위로 변환
		m_totalTime = duration<float>(curTime - m_startTime).count();

		m_prevTime = curTime;
	}
	float GameTimer::GetDeltaTime() const
	{
		return m_deltaTime;
	}
	float GameTimer::GetTotalTime() const
	{
		return m_totalTime;
	}
}