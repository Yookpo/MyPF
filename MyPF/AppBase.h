#pragma once
#include "Renderer.h"
#include "GameTimer.h"
#include "InputSystem.h"
#include "Scene.h"
#include "Camera.h"
#include "DirectionalLight.h"
#include "GraphicsDevice.h"
#include "GraphicsResourceManager.h"
#include "AssetManager.h"

// ImGui 사용에 필요한 헤더파일
#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

// ImGui 용 WndProcHandler
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace My
{
	enum class AppMode
	{
		Editor,
		Play,
	};

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
		bool  InitMainWindow();
		bool  InitGUI();
		float GetAspectRatio(float sceneViewWidth, float sceneViewHeight) const;
		void  EnterPlayMode();
		void  ExitPlayMode();
		void  CenterCursorInSceneView();

		bool InitGreyBoxScene();
		bool CanInteractWithPowerSwitch() const;
		void ActivatePower();

	public:
		HWND  m_mainWindow;
		int	  m_screenWidth; // 렌더링할 최종 화면의 해상도
		int	  m_screenHeight;
		float m_guiWidth = 0;

		AppMode m_appMode;

		GraphicsDevice			m_graphicsDevice;
		GraphicsResourceManager m_resourceManager;
		AssetManager			m_assetManager;
		Renderer				m_renderer;
		GameTimer				m_gameTimer;
		InputSystem				m_inputSystem;

		Scene		m_scene;
		GameObject* m_selectedObject;
		GameObject* m_powerSwitchObject;

		Camera m_camera;
		Camera m_editorCameraSnapshot;
		float  m_cameraSpeed;
		float  m_mouseSensitivity;
		float  m_interactionRange;
		float  m_interactionFacingThreshold;
		bool   m_isPowerOn;

		DirectionalLight m_directionalLight;

		std::array<float, 4> m_backgroundColor;
	};

} // namespace My
