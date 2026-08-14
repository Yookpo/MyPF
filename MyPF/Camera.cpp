#include "Camera.h"

namespace My
{
	Camera::Camera()
	{
	}
	void Camera::SetAspectRatio(float newAspectRatio)
	{
		if (newAspectRatio <= 0.0f)
		{
			return;
		}
		m_aspectRatio = newAspectRatio;
	}
	Matrix Camera::GetViewMatrix() const
	{
		return XMMatrixLookToLH(m_position, m_forward, m_up);
	}
	Matrix Camera::GetProjectionMatrix() const
	{
		using namespace DirectX;

		if (m_usePerspectiveProjection)
		{
			return XMMatrixPerspectiveFovLH(
				XMConvertToRadians(m_fovAngleY), m_aspectRatio, m_nearZ, m_farZ
			);
		}
		else
		{
			return XMMatrixOrthographicOffCenterLH(-m_aspectRatio, m_aspectRatio, -1.0f, 1.0f, m_nearZ, m_farZ);
		}
	}
}




