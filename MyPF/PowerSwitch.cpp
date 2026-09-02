#include "PowerSwitch.h"
#include "GameObject.h"

namespace My
{
	PowerSwitch::PowerSwitch()
		: m_gameObject{ nullptr }, m_interactionRange{ 2.0f }, m_interactionFacingThreshold{ 0.8f }, m_isPowerOn{ false }
	{
	}
	void PowerSwitch::Initialize(GameObject& gameObject)
	{
		m_gameObject = &gameObject;
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

} // namespace My
