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

		// 꺼진 골목에서 스위치를 찾을 수 있게 하는 대기 표시등. 번지지 않을 만큼만 밝다.
		Vector3 m_powerOffEmissiveColor;
		Vector3 m_powerOnEmissiveColor;
		float	m_powerOffEmissiveIntensity;
		float	m_powerOnEmissiveIntensity;

		float m_interactionRange;
		bool  m_isPowerOn;
		bool  m_isHighlighted;
	};
} // namespace My
