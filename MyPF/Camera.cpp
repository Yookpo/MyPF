#include "Camera.h"
#include <algorithm>

namespace My
{
	Camera::Camera()
	{
		UpdateForward();
	}

	void Camera::UpdateForward()
	{
		const float radYaw = DirectX::XMConvertToRadians(m_yaw);
		const float radPitch = DirectX::XMConvertToRadians(m_pitch);

		Vector3 newForward = {
			cos(radPitch) * sin(radYaw),
			sin(radPitch),
			cos(radPitch) * cos(radYaw)
		};
		m_forward = newForward;
		m_forward.Normalize();
	}

	void Camera::SetYawPitch(const float newYaw, const float newPitch)
	{
		m_yaw = newYaw;
		m_pitch = std::clamp(newPitch, -89.0f, 89.0f);

		UpdateForward();
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




