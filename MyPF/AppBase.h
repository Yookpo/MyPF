#pragma once
#include <iostream>
#include <vector>
#include <windows.h>

namespace My
{
	using std::vector;
	using std::wstring;


	class AppBase
	{
	public:
		AppBase();
		virtual ~AppBase();

		int Run();

		virtual bool Initialize();
		virtual void Update(float dt);
		virtual void Render();

		virtual LRESULT MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


	protected:
		bool InitMainWindow();

	public:
		int m_screenWidth;	// 렌더링할 최종 화면의 해상도
		int m_screenHeight;
		HWND m_mainWindow;
		UINT numQualityLevels;


	};

}

