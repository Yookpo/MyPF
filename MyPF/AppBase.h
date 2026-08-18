#pragma once
#include "Renderer.h"
#include "GameTimer.h"
#include "Scene.h"
#include "Mesh.h"
#include "Camera.h"
#include "DirectionalLight.h"

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

		void UpdateGui();

		virtual LRESULT MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


	protected:
		bool InitMainWindow();
		bool InitGUI();
		float GetAspectRatio(float sceneViewWidth, float sceneViewHeight) const;

	public:
		HWND m_mainWindow;
		int m_screenWidth;	// 렌더링할 최종 화면의 해상도
		int m_screenHeight;
		float m_guiWidth = 0;

		Renderer m_renderer;
		GameTimer m_gameTimer;
		Scene m_scene;
		GameObject* m_selectedObject;
		Mesh m_cubeMesh;
		Mesh m_triangleMesh;

		Camera m_camera;

		DirectionalLight m_directionalLight;

		std::array<float, 4> m_backgroundColor;
	};

}



