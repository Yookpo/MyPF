#include "PowerSwitch.h"
#include "GameObject.h"
#include "Material.h"
#include <DirectXCollision.h>

namespace My
{
	using namespace DirectX;
	using namespace DirectX::SimpleMath;

	PowerSwitch::PowerSwitch()
		: m_gameObject{ nullptr }
		, m_powerOffColor{ 0.30f, 0.12f, 0.10f }
		, m_powerOnColor{ 0.22f, 0.85f, 0.55f }
		, m_powerOffEmissiveColor{ 0.90f, 0.12f, 0.08f }
		, m_powerOnEmissiveColor{ 0.25f, 1.0f, 0.55f }
		, m_powerOffEmissiveIntensity{ 0.20f }
		, m_powerOnEmissiveIntensity{ 0.30f }
		, m_interactionRange{ 2.0f }
		, m_isPowerOn{ false }
		, m_isHighlighted{ false }
	{
	}
	void PowerSwitch::Initialize(GameObject& gameObject)
	{
		if (!gameObject.GetMeshComponent().HasMaterial())
		{
			return;
		}

		m_gameObject = &gameObject;
		Material* powerSwitchMat = m_gameObject->GetMeshComponent().GetMaterial();
		// 골목이 청록 계열이라 강조도 청록이면 네온에 묻힌다. 보색인 호박색을 쓴다.
		powerSwitchMat->SetRimColor(Vector3(1.0f, 0.75f, 0.25f));
		powerSwitchMat->SetRimPower(4.0f);

		ApplyVisualState();
	}

	bool PowerSwitch::IsPowerOn() const
	{
		return m_isPowerOn;
	}

	bool PowerSwitch::Toggle()
	{
		if (!m_gameObject)
		{
			return false;
		}

		m_isPowerOn = !m_isPowerOn;
		ApplyVisualState();

		return true;
	}

	bool PowerSwitch::CanInteract(const Vector3& viewerPosition, const Vector3& viewerForward) const
	{
		if (!m_gameObject)
		{
			return false;
		}

		DirectX::BoundingBox boundingBox(Vector3(0.0f), Vector3(0.5f));
		Matrix				 worldMat = m_gameObject->GetTransform().GetWorldMatrix();
		boundingBox.Transform(boundingBox, worldMat);

		// 카메라의 위치와 forward로 Ray 생성
		SimpleMath::Ray interactionRay = SimpleMath::Ray(viewerPosition, viewerForward);
		float			hitDistance = 0.0f;
		if (interactionRay.Intersects(boundingBox, hitDistance))
		{
			// 상호작용 범위 내에 있어야 상호작용 가능
			if (hitDistance <= m_interactionRange)
			{
				return true;
			}
		}

		return false;
	}

	void PowerSwitch::ApplyVisualState()
	{
		if (!m_gameObject || !m_gameObject->GetMeshComponent().HasMaterial())
		{
			return;
		}

		auto powerSwitchMat = m_gameObject->GetMeshComponent().GetMaterial();

		// 켜져 있다면 OnColor 적용
		if (m_isPowerOn)
		{
			powerSwitchMat->SetBaseColor(m_powerOnColor);
			powerSwitchMat->SetEmissiveColor(m_powerOnEmissiveColor);
			powerSwitchMat->SetEmissiveIntensity(m_powerOnEmissiveIntensity);
		}
		else
		{
			powerSwitchMat->SetBaseColor(m_powerOffColor);
			powerSwitchMat->SetEmissiveColor(m_powerOffEmissiveColor);
			powerSwitchMat->SetEmissiveIntensity(m_powerOffEmissiveIntensity);
		}

		// 상호작용거리가 된다면 Rim Intensity 적용
		if (m_isHighlighted)
		{
			powerSwitchMat->SetRimIntensity(3.0f);
		}
		else
		{
			powerSwitchMat->SetRimIntensity(0.0f);
		}

		return;
	}

	void PowerSwitch::SetHighlighted(bool isHighlighted)
	{
		if (m_isHighlighted != isHighlighted)
		{
			m_isHighlighted = isHighlighted;
			ApplyVisualState();
		}
		return;
	}

	bool PowerSwitch::IsHighlighted() const
	{
		return m_isHighlighted;
	}

} // namespace My
