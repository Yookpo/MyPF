#pragma once
#include <array>
#include <Windows.h>

namespace My
{
	struct MouseDelta
	{
		int m_mouseDeltaX = 0;
		int m_mouseDeltaY = 0;
	};

	class InputSystem
	{
	public:
		InputSystem();
		void ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam);
		bool IsKeyDown(UINT key) const;

		MouseDelta ConsumeMouseDelta();
		void	   Reset();

	private:
		std::array<bool, 256> m_keyDown{}; // Win32 Virtual-Key 값을 인덱스

		// UI, Picking, 마우스 위치
		int	 m_mouseX = 0;
		int	 m_mouseY = 0;
		bool m_hasMousePosition = false;

		// 카메라 회전
		MouseDelta m_mouseDelta;
	};

} // namespace My
