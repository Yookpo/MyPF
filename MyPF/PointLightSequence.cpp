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
			pointLightEntry.emissiveMat->SetEmissiveMultiplier(1.0f);
			m_enabledLightCount += 1;
		}
		else
		{
			const auto&			 pointLightEntry = m_pointLightSequenceEntries[m_enabledLightCount - 1];
			GameObject*			 pointLightObject = pointLightEntry.pointLightObject;
			PointLightComponent& pointLightComponent = pointLightObject->GetPointLightComponent();
			pointLightComponent.SetEnabled(false);
			pointLightEntry.emissiveMat->SetEmissiveMultiplier(0.0f);
			m_enabledLightCount -= 1;
		}
	}

	bool PointLightSequence::AddSequenceEntry(GameObject& pointLightObject, Material* mat)
	{
		if (!mat || !pointLightObject.HasPointLightComponent())
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

		// 새로운 Entry 등록
		SequenceEntry entry{ &pointLightObject, mat };
		pointLightObject.GetPointLightComponent().SetEnabled(false);
		mat->SetEmissiveMultiplier(0.0f);

		m_pointLightSequenceEntries.emplace_back(entry);

		return true;
	}
} // namespace My
