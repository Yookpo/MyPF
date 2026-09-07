#include "PointLightSequence.h"
#include "GameObject.h"
#include "Material.h"

namespace My
{
	void PointLightSequence::Initialize()
	{
		m_elapsedTime = 0.0f;
		m_enabledLightCount = m_targetEnabledLightCount = 0;
		m_pointLightSequenceEntries.clear();
	}

	void PointLightSequence::Start(bool shouldEnable)
	{
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
		if (!IsPlaying())
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
			const auto&			 pointLightEntry = m_pointLightSequenceEntries[m_enabledLightCount];
			GameObject*			 pointLightObject = pointLightEntry.pointLightObject;
			PointLightComponent& pointLightComponent = pointLightObject->GetPointLightComponent();
			pointLightComponent.SetEnabled(true);
			pointLightEntry.emissiveMat->SetEmissiveIntensity(pointLightEntry.emissiveIntensity);
			m_enabledLightCount += 1;
		}
		else
		{
			const auto&			 pointLightEntry = m_pointLightSequenceEntries[m_enabledLightCount - 1];
			GameObject*			 pointLightObject = pointLightEntry.pointLightObject;
			PointLightComponent& pointLightComponent = pointLightObject->GetPointLightComponent();
			pointLightComponent.SetEnabled(false);
			pointLightEntry.emissiveMat->SetEmissiveIntensity(0);
			m_enabledLightCount -= 1;
		}
	}

	bool PointLightSequence::AddSequenceEntry(GameObject& pointLightObject, Material* mat, float intensity)
	{
		if (!mat || !pointLightObject.HasPointLightComponent() || intensity <= 0.0f)
		{
			return false;
		}

		// 기존 Entry 순회 중복 검사
		for (const auto& sequenceEntry : m_pointLightSequenceEntries)
		{
			// 등록 할 포인트라이트가 이미 있다면 패싱
			if (sequenceEntry.pointLightObject == &pointLightObject || sequenceEntry.emissiveMat == mat)
			{
				return false;
			}
		}

		SequenceEntry entry{ &pointLightObject, mat, intensity };
		pointLightObject.GetPointLightComponent().SetEnabled(false);
		mat->SetEmissiveIntensity(0.0f);

		m_pointLightSequenceEntries.emplace_back(entry);

		return true;
	}
} // namespace My
