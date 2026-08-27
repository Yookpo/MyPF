#include "InputSystem.h"
#include <windowsx.h>

namespace My
{
	InputSystem::InputSystem()
	{
		Reset();
	}

	// 실제 키 상태 저장
	void InputSystem::ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
			case WM_KEYDOWN:
				if (wParam < m_keyDown.size())
				{
					m_keyDown[wParam] = true;
				}
				break;

			case WM_KEYUP:
				if (wParam < m_keyDown.size())
				{
					m_keyDown[wParam] = false;
				}
				break;

			case WM_MOUSEMOVE:
			{
				int curX = GET_X_LPARAM(lParam);
				int curY = GET_Y_LPARAM(lParam);
				if (m_hasMousePosition)
				{
					m_mouseDelta.m_mouseDeltaX += (curX - m_mouseX);
					m_mouseDelta.m_mouseDeltaY += (curY - m_mouseY);
				}
				m_mouseX = curX;
				m_mouseY = curY;
				m_hasMousePosition = true;
				break;
			}

			case WM_KILLFOCUS:
				Reset();
				break;
		}
	}

	bool InputSystem::IsKeyDown(UINT key) const
	{
		if (key >= m_keyDown.size())
		{
			return false;
		}

		return m_keyDown[key];
	}

	MouseDelta InputSystem::ConsumeMouseDelta()
	{
		MouseDelta delta = m_mouseDelta;
		m_mouseDelta.m_mouseDeltaX = m_mouseDelta.m_mouseDeltaY = 0;

		return delta;
	}

	// Window가 포커스를 잃었을 때
	// Editor에서 Play로 전환될 때
	// Play에서 Stop으로 돌아올 때
	void InputSystem::Reset()
	{
		m_keyDown.fill(false);

		m_mouseDelta.m_mouseDeltaX = m_mouseDelta.m_mouseDeltaY = 0;
		m_mouseX = m_mouseY = 0;
		m_hasMousePosition = false;
	}

} // namespace My
