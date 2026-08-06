#pragma once
#include "Renderer.h"

// ImGui 사용에 필요한 헤더파일
#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

// ImGui 용 WndProcHandler
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

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

		void UpdateUI();

		virtual LRESULT MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


	protected:
		bool InitMainWindow();
		bool InitGUI();

	public:
		HWND m_mainWindow;
		int m_screenWidth;	// 렌더링할 최종 화면의 해상도
		int m_screenHeight;

		Renderer m_renderer;
	};

}

