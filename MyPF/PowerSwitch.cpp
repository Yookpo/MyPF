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
		, m_powerOffColor{ 0.35f, 0.05f, 0.05f }
		, m_powerOnColor{ 0.15f, 0.8f, 0.25f }
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
		powerSwitchMat->SetRimColor(Vector3(0.3f, 0.9f, 1.0f));
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
		}
		else
		{
			powerSwitchMat->SetBaseColor(m_powerOffColor);
		}

		// 상호작용거리가 된다면 Rim Intensity 적용
		if (m_isHighlighted)
		{
			powerSwitchMat->SetRimIntensity(2.5f);
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
