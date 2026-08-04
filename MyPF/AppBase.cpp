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
		switch (msg)
		{
		case WM_DESTROY:
			m_mainWindow = nullptr;
			::PostQuitMessage(0);
			return 0;
		}

		return ::DefWindowProcW(hWnd, msg, wParam, lParam);
	}

	AppBase::AppBase()
		: m_screenWidth(1280), m_screenHeight(720),
		m_mainWindow(nullptr), m_renderer{}
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
	}


	void AppBase::Update(float dt) {}
	void AppBase::Render()
	{
		std::array<float, 4> cyberPunkColor = { 0.047f, 0.031f, 0.125f, 1.0f };

		m_renderer.BeginFrame(cyberPunkColor);

		if (!m_renderer.EndFrame())
		{
			OutputDebugStringW(L"Rendering failed, Program shutting down");
			PostQuitMessage(-1);
		}
	}

	int AppBase::Run()
	{
		// Main Message loop
		MSG msg = { 0 };
		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				Update(1.0f);
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
}