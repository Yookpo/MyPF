#pragma once
#include <array>
#include <Windows.h>

namespace My
{
	class InputSystem
	{
	public:
		InputSystem();
		void ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam);
		bool IsKeyDown(UINT key) const;
		void Reset();

	private:
		std::array<bool, 256> m_keyDown{}; // Win32 Virtual-Key °ªÀ» ÀÎµ¦½º
	};

} // namespace My
