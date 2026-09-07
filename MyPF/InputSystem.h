#pragma once
#include <array>
#include <Windows.h>

namespace My
{
	struct MouseDelta
	{
		int deltaX = 0;
		int deltaY = 0;
	};

	class InputSystem
	{
	public:
		InputSystem();
		void ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam);
		bool IsKeyDown(UINT keyCode) const;
		bool WasKeyPressed(UINT keyCode) const;
		bool IsRightMouseButtonDown() const { return m_isRightMouseButtonDown; }
		void EndFrame();

		MouseDelta ConsumeMouseDelta();
		void	   SetMouseReferencePosition(int x, int y);
		void	   ResetMouseTracking();
		void	   Reset();

	private:
		std::array<bool, 256> m_keyDown{};	  // 지속 상태
		std::array<bool, 256> m_keyPressed{}; // 한 프레임 상태

		// UI, Picking, 마우스 위치
		int	 m_mouseX{ 0 };
		int	 m_mouseY{ 0 };
		bool m_hasMousePosition{ false };
		bool m_isRightMouseButtonDown{ false };

		// 카메라 회전
		MouseDelta m_mouseDelta;
	};

} // namespace My
