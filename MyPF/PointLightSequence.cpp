#include "PointLightSequence.h"
#include "Scene.h"
#include "Material.h"

namespace My
{
	void PointLightSequence::Initialize(Scene& scene)
	{
		m_scene = &scene;
		m_elapsedTime = 0.0f;
		m_enabledLightCount = m_targetEnabledLightCount = 0;
		m_pointLightSequenceEntries.clear();
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
			m_targetEnabledLightCount = m_pointLightSequenceEntries.size();
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

		// Entry를 가져옴

		if (m_enabledLightCount < m_targetEnabledLightCount)
		{
			const auto& pointLightEntry = m_pointLightSequenceEntries[m_enabledLightCount];
			if (m_scene->SetPointLightEnabled(pointLightEntry.index, true))
			{
				pointLightEntry.emissiveMat->SetEmissiveIntensity(pointLightEntry.emissiveIntensity);
				m_enabledLightCount += 1;
			}
		}
		else
		{
			const auto& pointLightEntry = m_pointLightSequenceEntries[m_enabledLightCount - 1];
			if (m_scene->SetPointLightEnabled(pointLightEntry.index, false))
			{
				pointLightEntry.emissiveMat->SetEmissiveIntensity(0.0f);
				m_enabledLightCount -= 1;
			}
		}
	}

	bool PointLightSequence::AddSequenceEntry(std::size_t pointLightIndex, Material* mat, float intensity)
	{
		if (!m_scene || !mat || pointLightIndex >= m_scene->GetPointLightCount() || intensity <= 0.0f)
		{
			return false;
		}

		// 기존 Entry 순회 중복 검사
		for (const auto& sequenceEntry : m_pointLightSequenceEntries)
		{
			// 등록 할 포인트라이트가 이미 있다면 패싱
			if (sequenceEntry.index == pointLightIndex || sequenceEntry.emissiveMat == mat)
			{
				return false;
			}
		}

		SequenceEntry entry{ pointLightIndex, mat, intensity };
		if (!m_scene->SetPointLightEnabled(pointLightIndex, false))
		{
			return false;
		}
		mat->SetEmissiveIntensity(0.0f);

		m_pointLightSequenceEntries.emplace_back(entry);

		return true;
	}
} // namespace My