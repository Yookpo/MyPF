#include "PointLightSequence.h"
#include "Scene.h"

namespace My
{
	void PointLightSequence::Initialize(Scene& scene)
	{
		m_scene = &scene;
		m_elapsedTime = 0.0f;
		m_enabledLightCount = m_targetEnabledLightCount = 0;
		m_scene->SetAllPointLightsEnabled(false);
	}

	void PointLightSequence::Start(bool shouldEnable)
	{
		if (!m_scene)
		{
			return;
		}

		m_elapsedTime = 0.0f;

		// 목표 개수 0개로 설정
		if (!shouldEnable)
		{
			m_targetEnabledLightCount = 0;
		}
		else
		{
			m_targetEnabledLightCount = m_scene->GetPointLightCount();
		}
	}

	void PointLightSequence::Update(float deltaTime)
	{
		if (!m_scene || !IsPlaying())
		{
			return;
		}

		m_elapsedTime += deltaTime;
		if (m_elapsedTime < m_stepInterval)
		{
			return;
		}

		m_elapsedTime -= m_stepInterval;

		if (m_enabledLightCount < m_targetEnabledLightCount)
		{
			if (m_scene->SetPointLightEnabled(m_enabledLightCount, true))
			{
				m_enabledLightCount += 1;
			}
		}
		else
		{
			if (m_scene->SetPointLightEnabled(m_enabledLightCount - 1, false))
			{
				m_enabledLightCount -= 1;
			}
		}
	}

} // namespace My