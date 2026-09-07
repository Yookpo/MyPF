#pragma once

namespace My
{
	class Camera;
	class InputSystem;

	class FirstPersonCameraController
	{
	public:
		FirstPersonCameraController();
		void  Initialize(Camera& camera, InputSystem& inputSystem);
		void  Update(float deltaTime);
		void  SetMoveSpeed(float newMoveSpeed) { m_moveSpeed = newMoveSpeed; }
		float GetMoveSpeed() const { return m_moveSpeed; }
		void  SetMouseSensitivity(float newMouseSensitivity) { m_mouseSensitivity = newMouseSensitivity; }
		float GetMouseSensitivity() const { return m_mouseSensitivity; }

	private:
		void UpdateMovement(float deltaTime);
		void UpdateRotation();

	private:
		Camera*		 m_camera;
		InputSystem* m_inputSystem;
		float		 m_moveSpeed;
		float		 m_mouseSensitivity;
	};
} // namespace My
