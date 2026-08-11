#include "AppBase.h"

namespace My
{
	using namespace std;

	// RegisterClassEx()에서 멤버 함수를 직접 등록할 수가 없기 때문에
	// 클래스의 멤버 함수에서 간접적으로 메시지를 처리할 수 있도록 도와줍니다.
	AppBase* g_appBase = nullptr;

	// RegisterClassEx()에서 실제로 등록될 콜백 함수
	LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// g_appBase를 이용해서 간접적으로 멤버 함수 호출
		if (g_appBase == nullptr)
		{
			return DefWindowProcW(hWnd, msg, wParam, lParam);
		}

		return g_appBase->MsgProc(hWnd, msg, wParam, lParam);
	}

	// 윈도우 프로시저
	// Windows가 전달하는 창 이벤트 처리
	LRESULT AppBase::MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		{
			return true;
		}

		switch (msg)
		{
		case WM_SIZE:
			m_screenWidth = int(LOWORD(lParam));
			m_screenHeight = int(HIWORD(lParam));
			m_guiWidth = 0;

			m_renderer.Resize(m_screenWidth, m_screenHeight);
			break;

		case WM_DESTROY:
			m_mainWindow = nullptr;
			::PostQuitMessage(0);
			return 0;
		}

		return ::DefWindowProcW(hWnd, msg, wParam, lParam);
	}

	AppBase::AppBase()
		: m_screenWidth(1280), m_screenHeight(720),
		m_mainWindow(nullptr), m_renderer{}, m_backgroundColor{ 0.047f, 0.031f, 0.125f, 1.0f },
		m_selectedObject{ nullptr }
	{
		g_appBase = this;
	}

	AppBase::~AppBase()
	{
		if (m_mainWindow)
		{
			DestroyWindow(m_mainWindow);
		}

		g_appBase = nullptr;

		// ImGui 소멸
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}


	void AppBase::Update(float dt) {}
	void AppBase::Render()
	{
		m_renderer.BeginFrame(m_backgroundColor);

		const auto& sceneObjects = m_scene.GetGameObjects();
		for (const auto& obj : sceneObjects)
		{
			const Transform& tr = obj->GetTransform();
			const Matrix world = tr.GetWorldMatrix();

			if (!m_renderer.DrawCube(world))
			{
				OutputDebugStringW(L"Draw Cube failed, Program shutting down");
				PostQuitMessage(-1);
				return;
			}
		}

		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		if (!m_renderer.EndFrame())
		{
			OutputDebugStringW(L"Rendering failed, Program shutting down");
			PostQuitMessage(-1);
		}
	}

	void AppBase::UpdateUI()
	{
		// ImGui 로직
		// 이후 ImGui UI 컨트롤 추가는 ImGui::NewFrame()과 ImGui::Render() 사이인 여기에 위치
		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::Begin("Test Window");
		ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		if (ImGui::ColorEdit4("Control BackColor", m_backgroundColor.data(), 0))
		{
		}

		if (m_selectedObject)
		{
			Transform& tr = m_selectedObject->GetTransform();
			// 위치 수정
			Vector3 pos = tr.GetPosition();
			if (ImGui::DragFloat3("Move", &pos.x, 0.01f, -1.0f, 1.0f))
			{
				tr.SetPosition(pos);
			}
			// 회전 수정
			Vector3 rot = tr.GetRotation();
			if (ImGui::SliderFloat3("Rotate(Rad)", &rot.x, -3.14f, 3.14f))
			{
				tr.SetRotation(rot);
			}
			// 스케일 수정
			Vector3 scale = tr.GetScale();
			if (ImGui::SliderFloat3("Scaling", &scale.x, 0.1f, 2.0f))
			{
				tr.SetScale(scale);
			}
		}

		m_guiWidth = ImGui::GetWindowSize().x;

		ImGui::End();
	}

	int AppBase::Run()
	{
		// Main Message loop
		MSG msg = { 0 };
		m_gameTimer.Reset();

		// 기존에 guiWidth 변화없다면 뷰포트 변화안함
		static float previousGuiWidth = m_guiWidth;

		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				m_gameTimer.Tick();

				// ImGui 프레임 시작
				// 백버퍼 렌더링 호출 전에 ImGui 렌더링 준비, 컨트롤 설정, 렌더링 요청 함수 호출
				ImGui_ImplWin32_NewFrame();
				ImGui_ImplDX11_NewFrame();
				ImGui::NewFrame();

				UpdateUI();

				ImGui::Render();

				if (previousGuiWidth != m_guiWidth)
				{
					previousGuiWidth = m_guiWidth;
					m_renderer.SetSceneViewport(m_guiWidth, 0, m_screenWidth - m_guiWidth, m_screenHeight);
				}


				Update(m_gameTimer.GetDeltaTime());

				Render();
			}
		}

		return static_cast<int>(msg.wParam);
	}


	bool AppBase::Initialize()
	{
		if (!InitMainWindow())
			return false;

		if (!m_renderer.Initialize(m_mainWindow, m_screenWidth, m_screenHeight))
			return false;

		if (!InitGUI())
			return false;

		m_selectedObject = &m_scene.CreateGameObject("cube");
		m_selectedObject->GetTransform().SetScale(Vector3(0.5f, 0.5f, 0.5f));

		return true;
	}

	bool AppBase::InitMainWindow()
	{
		// 창 클래스 등록
		WNDCLASSEX wc =
		{
			sizeof(WNDCLASSEX),CS_CLASSDC,
			WndProc,
			0L,0L,
			GetModuleHandle(NULL),
			NULL,
			LoadCursor(nullptr, IDC_ARROW),
			NULL,
			NULL,
			L"CyberPunk",
			NULL
		};

		if (!RegisterClassEx(&wc))
		{
			std::cerr << "RegisterClassEx() failed." << endl;
			return false;
		}

		RECT wr = { 0,0, m_screenWidth,m_screenHeight };

		AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, false);

		m_mainWindow = CreateWindow(
			wc.lpszClassName, L"CyberPunk",
			WS_OVERLAPPEDWINDOW,
			100,	// 윈도우 좌측 상단의 x 좌표
			100,	// 윈도우 좌측 상단의 y 좌표
			wr.right - wr.left,	 // 윈도우 가로 방향 해상도
			wr.bottom - wr.top,	 // 윈도우 세로 방향 해상도
			NULL, NULL, wc.hInstance, NULL);

		if (!m_mainWindow)
		{
			std::cerr << "CreateWindow() failed.\n";
			return false;
		}

		ShowWindow(m_mainWindow, SW_SHOWDEFAULT);
		UpdateWindow(m_mainWindow);

		return true;
	}
	bool AppBase::InitGUI()
	{
		// ImGui 생성 및 초기화
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();

		// Setup Platform/Renderer backends
		if (!ImGui_ImplDX11_Init(m_renderer.GetDevice().Get(), m_renderer.GetContext().Get())) {
			return false;
		}

		if (!ImGui_ImplWin32_Init((void*)m_mainWindow)) {
			return false;
		}

		return true;
	}
}