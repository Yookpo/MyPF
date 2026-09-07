#pragma once
#include <directxtk/SimpleMath.h>

namespace My
{
	using DirectX::SimpleMath::Vector3;

	class PointLightComponent
	{
	public:
		Vector3 GetColor() const { return m_color; }
		void	SetColor(const Vector3& newColor) { m_color = newColor; }
		float	GetIntensity() const { return m_intensity; }
		void	SetIntensity(float newIntensity) { m_intensity = newIntensity; }
		float	GetRange() const { return m_range; }
		void	SetRange(float newRange) { m_range = newRange; }
		bool	IsEnabled() const { return m_isEnabled; }
		void	SetEnabled(bool enabled) { m_isEnabled = enabled; }

	private:
		Vector3 m_color{ 1.0f };
		float	m_intensity{ 1.0f };
		float	m_range{ 2.0f };
		bool	m_isEnabled{ false };
	};

} // namespace My
