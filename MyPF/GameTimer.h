#pragma once
// 시간 측정용
#include <chrono>

namespace My
{
	using namespace std::chrono;

	class GameTimer
	{
	public:
		GameTimer();
		void Reset();
		void Tick();
		float GetDeltaTime() const;
		float GetTotalTime() const;

	private:
		time_point<steady_clock> m_startTime;	// 시작 시간
		time_point<steady_clock> m_prevTime;	// 직전 프레임 시간
		float m_deltaTime;	// 마지막으로 계산한 시간
		float m_totalTime;	// 시작 이후 누적된 시간

	};
}





