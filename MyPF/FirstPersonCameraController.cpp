#include "FirstPersonCameraController.h"
#include "Camera.h"
#include "InputSystem.h"

namespace My
{
	FirstPersonCameraController::FirstPersonCameraController()
		: m_camera{ nullptr }, m_inputSystem{ nullptr }, m_moveSpeed{ 4.0f }, m_mouseSensitivity{ 0.1f }
	{
	}

	void FirstPersonCameraController::Initialize(Camera& camera, InputSystem& inputSystem)
	{
		m_camera = &camera;
		m_inputSystem = &inputSystem;
	}

	void FirstPersonCameraController::Update(float deltaTime)
	{
		if (!m_camera || !m_inputSystem)
		{
			return;
		}

		this->UpdateRotation();
		this->UpdateMovement(deltaTime);
	}

	void FirstPersonCameraController::UpdateMovement(float deltaTime)
	{
		Vector3 cameraPos = m_camera->GetPosition();
		Vector3 cameraForward = m_camera->GetForward();
		Vector3 cameraUp = m_camera->GetUp();
		cameraForward.y = 0.0f;

		if (cameraForward.LengthSquared() > 0.00001f)
		{
			cameraForward.Normalize();
		}

		Vector3 cameraRight = cameraUp.Cross(cameraForward);
		if (cameraRight.LengthSquared() > 0.00001f)
		{
			cameraRight.Normalize();
		}

		Vector3 moveDirection{ 0.0f };

		if (m_inputSystem->IsKeyDown('W'))
		{
			moveDirection += cameraForward;
		}

		if (m_inputSystem->IsKeyDown('S'))
		{
			moveDirection -= cameraForward;
		}

		if (m_inputSystem->IsKeyDown('A'))
		{
			moveDirection -= cameraRight;
		}

		if (m_inputSystem->IsKeyDown('D'))
		{
			moveDirection += cameraRight;
		}

		if (moveDirection.LengthSquared() > 0.00001f)
		{
			moveDirection.Normalize();
		}

		cameraPos += moveDirection * m_moveSpeed * deltaTime;

		m_camera->SetPosition(cameraPos);
	}

	void FirstPersonCameraController::UpdateRotation()
	{
		MouseDelta delta = m_inputSystem->ConsumeMouseDelta();
		float	   cameraYaw = m_camera->GetYaw();
		float	   cameraPitch = m_camera->GetPitch();

		cameraYaw += (delta.deltaX * m_mouseSensitivity);
		cameraPitch -= (delta.deltaY * m_mouseSensitivity);

		m_camera->SetYawPitch(cameraYaw, cameraPitch);
	}

} // namespace My
