#pragma once
#include "Renderer.h"

namespace My
{
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
		HWND m_mainWindow;
		int m_screenWidth;	// 렌더링할 최종 화면의 해상도
		int m_screenHeight;

		Renderer m_renderer;
	};

}

