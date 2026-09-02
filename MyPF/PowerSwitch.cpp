#include "PowerSwitch.h"
#include "GameObject.h"
#include "Material.h"

namespace My
{
	PowerSwitch::PowerSwitch()
		: m_gameObject{ nullptr }, m_powerOffColor{ 0.35f, 0.05f, 0.05f }, m_powerOnColor{ 0.15f, 0.8f, 0.25f }, m_interactionRange{ 2.0f }, m_interactionFacingThreshold{ 0.8f }, m_isPowerOn{ false }
	{
	}
	void PowerSwitch::Initialize(GameObject& gameObject)
	{
		m_gameObject = &gameObject;
		ApplyVisualState();
	}

	bool PowerSwitch::IsPowerOn() const
	{
		return m_isPowerOn;
	}

	void PowerSwitch::Activate()
	{
		if (!m_gameObject || m_isPowerOn)
		{
			return;
		}

		m_isPowerOn = true;
		ApplyVisualState();
	}

	bool PowerSwitch::CanInteract(const Vector3& viewerPosition, const Vector3& viewerForward) const
	{
		if (!m_gameObject)
		{
			return false;
		}

		const Vector3 switchPosition = m_gameObject->GetTransform().GetPosition();

		Vector3 toSwitch = switchPosition - viewerPosition;

		const float distanceSquared = toSwitch.LengthSquared();
		const float interactionRangeSquared = m_interactionRange * m_interactionRange;

		if (distanceSquared > interactionRangeSquared)
		{
			return false;
		}

		if (distanceSquared < 0.00001f)
		{
			return false;
		}

		toSwitch.Normalize();

		const float facingDot = toSwitch.Dot(viewerForward);

		if (facingDot < m_interactionFacingThreshold)
		{
			return false;
		}

		return true;
	}

	void PowerSwitch::ApplyVisualState()
	{
		if (!m_gameObject || !m_gameObject->GetMeshComponent().HasMaterial())
		{
			return;
		}

		// 꺼져있다면 offColor 적용
		auto powerSwitchMat = m_gameObject->GetMeshComponent().GetMaterial();

		if (!m_isPowerOn)
		{
			powerSwitchMat->SetBaseColor(m_powerOffColor);
			return;
		}

		// 켜져 있다면 OnColor 적용
		powerSwitchMat->SetBaseColor(m_powerOnColor);
		return;
	}

} // namespace My
