#include "Transform.h"

namespace My
{
	Transform::Transform()
		: m_position(0.0f), m_rotation(0.0f), m_scale(1.0f)
	{
	}

	Matrix Transform::GetWorldMatrix() const
	{
		return (Matrix::CreateScale(m_scale) * 
			Matrix::CreateRotationY(m_rotation.y) * 
			Matrix::CreateRotationX(m_rotation.x) * 
			Matrix::CreateRotationZ(m_rotation.z) * 
			Matrix::CreateTranslation(m_position));
	}
} // namespace My
