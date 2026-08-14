#pragma once
#include <directxtk/SimpleMath.h>

namespace My
{
	using DirectX::SimpleMath::Vector3;
	using DirectX::SimpleMath::Matrix;

	class Camera
	{
	public:
		Camera();
		const Vector3& GetPosition() const { return m_position; }
		const Vector3& GetForward() const { return m_forward; }
		const Vector3& GetUp() const { return m_up; }

		void SetPosition(const Vector3& newPos) { m_position = newPos; }
		void SetForward(const Vector3& newFwd) { m_forward = newFwd; }
		void SetUp(const Vector3& newUp) { m_up = newUp; }

		bool IsPerspective() const { return m_usePerspectiveProjection; }
		float GetFovAngleY() const { return m_fovAngleY; }
		float GetAspectRatio() const { return m_aspectRatio; }
		float GetNearZ() const { return m_nearZ; }
		float GetFarZ() const { return m_farZ; }

		void SetAspectRatio(float newAspectRatio);

		Matrix GetViewMatrix() const;
		Matrix GetProjectionMatrix() const;

	private:
		bool m_usePerspectiveProjection = true;
		Vector3 m_position = { 0.0f, 0.0f, -2.0f };
		Vector3 m_forward = { 0.0f, 0.0f, 1.0f };
		Vector3 m_up = { 0.0f, 1.0f, 0.0f };
		float m_fovAngleY = 70.0f;
		float m_nearZ = 0.01f;
		float m_farZ = 100.0f;
		float m_aspectRatio = 1.0f;
	};

}




