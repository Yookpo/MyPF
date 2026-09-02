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
		void Activate();
		bool CanInteract(const Vector3& position, const Vector3& forward) const;

	private:
		GameObject* m_gameObject;
		float		m_interactionRange;
		float		m_interactionFacingThreshold;
		bool		m_isPowerOn;
	};
} // namespace My
