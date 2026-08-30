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
					if (!m_keyDown[wParam])
					{
						m_keyPressed[wParam] = true;
					}

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
					m_mouseDelta.deltaX += (curX - m_mouseX);
					m_mouseDelta.deltaY += (curY - m_mouseY);
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

	bool InputSystem::IsKeyDown(UINT keyCode) const
	{
		if (keyCode >= m_keyDown.size())
		{
			return false;
		}

		return m_keyDown[keyCode];
	}

	bool InputSystem::WasKeyPressed(UINT keyCode) const
	{
		if (keyCode >= m_keyPressed.size())
		{
			return false;
		}

		return m_keyPressed[keyCode];
	}

	MouseDelta InputSystem::ConsumeMouseDelta()
	{
		MouseDelta delta = m_mouseDelta;
		m_mouseDelta.deltaX = m_mouseDelta.deltaY = 0;

		return delta;
	}

	void InputSystem::SetMouseReferencePosition(int x, int y)
	{
		m_mouseDelta.deltaX = m_mouseDelta.deltaY = 0;
		m_mouseX = x;
		m_mouseY = y;
		m_hasMousePosition = true;
	}

	void InputSystem::EndFrame()
	{
		m_keyPressed.fill(false);
	}

	void InputSystem::ResetMouseTracking()
	{
		m_mouseDelta.deltaX = m_mouseDelta.deltaY = 0;
		m_mouseX = m_mouseY = 0;
		m_hasMousePosition = false;
	}

	// Window가 포커스를 잃었을 때
	// Editor에서 Play로 전환될 때
	// Play에서 Stop으로 돌아올 때
	void InputSystem::Reset()
	{
		m_keyDown.fill(false);
		m_keyPressed.fill(false);
		ResetMouseTracking();
	}

} // namespace My
