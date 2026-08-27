#include "InputSystem.h"

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

	// Window가 포커스를 잃었을 때
	// Editor에서 Play로 전환될 때
	// Play에서 Stop으로 돌아올 때
	void InputSystem::Reset()
	{
		m_keyDown.fill(false);
	}

} // namespace My
