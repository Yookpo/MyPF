#pragma once
#include <directxtk/SimpleMath.h>

namespace My
{
	class GameObject;
	using DirectX::SimpleMath::Vector3;

	class PowerSwitch
	{
	public:
		PowerSwitch();

		void Initialize(GameObject& gameObject);
		bool IsPowerOn() const;
		bool Toggle();
		void SetHighlighted(bool);
		bool IsHighlighted() const;
		bool CanInteract(const Vector3& position, const Vector3& forward) const;

	private:
		void ApplyVisualState();

	private:
		GameObject* m_gameObject;
		Vector3		m_powerOffColor;
		Vector3		m_powerOnColor;

		float m_interactionRange;
		bool  m_isPowerOn;
		bool  m_isHighlighted;
	};
} // namespace My
