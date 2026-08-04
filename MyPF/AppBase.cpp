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
		m_mainWindow(nullptr), numQualityLevels(0), m_screenViewport(D3D11_VIEWPORT())
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
		float cyberPunkColor[4] = { 0.047f, 0.031f, 0.125f, 1.0f };

		m_context->ClearRenderTargetView(m_renderTargetView.Get(), cyberPunkColor);
		m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);

		m_swapChain->Present(1, 0);
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
				Render();
			}
		}

		return 0;
	}


	bool AppBase::Initialize()
	{
		if (!InitMainWindow())
			return false;

		if (!InitDirect3D())
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

	bool AppBase::InitDirect3D()
	{
		// m_device, m_context 생성
		UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		ComPtr<ID3D11Device> device;
		ComPtr<ID3D11DeviceContext> context;

		const D3D_FEATURE_LEVEL featureLevels[2] = {
		D3D_FEATURE_LEVEL_11_0, // 더 높은 버전이 먼저 오도록 설정
		D3D_FEATURE_LEVEL_9_3 };
		D3D_FEATURE_LEVEL featureLevel;

		if (FAILED(D3D11CreateDevice(
			nullptr,    // Specify nullptr to use the default adapter.
			D3D_DRIVER_TYPE_HARDWARE, // Create a device using the hardware graphics driver.
			0, // Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
			createDeviceFlags, // Set debug and Direct2D compatibility flags.
			featureLevels,     // List of feature levels this app can support.
			ARRAYSIZE(featureLevels), // Size of the list above.
			D3D11_SDK_VERSION,     // Always set this to D3D11_SDK_VERSION for Microsoft Store apps.
			device.GetAddressOf(), // Returns the Direct3D device created.
			&featureLevel,         // Returns feature level of device created.
			context.GetAddressOf() // Returns the device immediate context.
		))) {
			std::cerr << "D3D11CreateDevice() failed." << endl;
			return false;
		}

		if (featureLevel != D3D_FEATURE_LEVEL_11_0) {
			std::cerr << "D3D Feature Level 11 unsupported." << endl;
			return false;
		}

		// 4X MSAA 지원하는지 확인
		device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &numQualityLevels);
		if (numQualityLevels <= 0) {
			std::cerr << "MSAA not supported." << endl;
		}

		if (FAILED(device.As(&m_device))) {
			std::cerr << "device.AS() failed." << endl;
			return false;
		}

		if (FAILED(context.As(&m_context))) {
			std::cerr << "context.As() failed." << endl;
			return false;
		}

		// swapchain 생성
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));

		sd.BufferDesc.Width = m_screenWidth;	  // set the back buffer width
		sd.BufferDesc.Height = m_screenHeight;	  // set the back buffer height
		sd.BufferDesc.RefreshRate.Numerator = 60; // use 32-bit color
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferCount = 2;	 // double buffering

		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = m_mainWindow; // the window to be used
		sd.Windowed = TRUE;             // windowed/full-screen mode
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // allow full-screen switching
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		if (numQualityLevels > 0)
		{
			sd.SampleDesc.Count = 4;	// how many multisamples
			sd.SampleDesc.Quality = numQualityLevels - 1;
		}
		else
		{
			sd.SampleDesc.Count = 1;
			sd.SampleDesc.Quality = 0;
		}

		if (FAILED(D3D11CreateDeviceAndSwapChain(
			0, // Default adapter
			D3D_DRIVER_TYPE_HARDWARE,
			0, // No software device
			createDeviceFlags, featureLevels, 1, D3D11_SDK_VERSION, &sd,
			m_swapChain.GetAddressOf(), m_device.GetAddressOf(), &featureLevel,
			m_context.GetAddressOf()))) {
			std::cerr << "D3D11CreateDeviceAndSwapChain() failed." << endl;
			return false;
		}

		if (!CreateRenderTargetView())
			return false;

		SetViewPort();

		// Create a rasterizer state
		D3D11_RASTERIZER_DESC rastDesc;
		ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC)); // Need this
		rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
		// rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
		rastDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
		rastDesc.FrontCounterClockwise = false;
		rastDesc.DepthClipEnable = true; // <- zNear, zFar 확인에 필요

		if (FAILED(m_device->CreateRasterizerState(&rastDesc,
			m_rasterizerState.GetAddressOf())))
		{
			std::cerr << "CreateRasterizerState() failed \n";
		}

		return true;
	}

	void AppBase::SetViewPort()
	{
		ZeroMemory(&m_screenViewport, sizeof(D3D11_VIEWPORT));
		m_screenViewport.TopLeftX = 0;
		m_screenViewport.TopLeftY = 0;

		m_screenViewport.Width = static_cast<float>(m_screenWidth);
		m_screenViewport.Height = static_cast<float>(m_screenHeight);
		m_screenViewport.MinDepth = 0.0f;
		m_screenViewport.MaxDepth = 1.0f;	// Note: important for depth buffering

		m_context->RSSetViewports(1, &m_screenViewport);
	}

	bool AppBase::CreateRenderTargetView()
	{
		ComPtr<ID3D11Texture2D> backBuffer;
		m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
		if (backBuffer)
		{
			m_device->CreateRenderTargetView(
				backBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf()
			);
		}
		else
		{
			std::cerr << "CreateRenderTargetView() failed\n";
			return false;
		}

		return true;
	}


}




