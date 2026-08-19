#include "GraphicsDevice.h"

namespace My
{
	bool GraphicsDevice::Initialize(HWND mainWindow, int screenWidth, int screenHeight)
	{
		if (!InitDirect3D(mainWindow, screenWidth, screenHeight))
		{
			return false;
		}

		if (!CreateRenderTargetView())
		{
			return false;
		}

		if (!CreateDepthBuffer(screenWidth, screenHeight))
		{
			return false;
		}

		return true;
	}

	bool GraphicsDevice::Resize(int screenWidth, int screenHeight)
	{
		// 창 최소화 상태이므로 작업 없이 넘어가기
		if (screenWidth <= 0 || screenHeight <= 0)
		{
			return true;
		}

		if (!m_device || !m_context || !m_swapChain)
		{
			OutputDebugStringW(L"No Device for resizing");
			return false;
		}

		// 현재 RTV/DSV 연결 해제
		m_context->OMSetRenderTargets(0, nullptr, nullptr);
		m_renderTargetView.Reset();
		m_depthStencilView.Reset();
		m_depthTexture.Reset();

		HRESULT hr = m_swapChain->ResizeBuffers(0,	// 현재 개수 유지
			screenWidth, screenHeight,	// 해상도 변경
			DXGI_FORMAT_UNKNOWN,	// 현재 포맷 유지
			DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
		);
		if (FAILED(hr))
		{
			return false;
		}
		if (!CreateRenderTargetView())
		{
			return false;
		}
		if (!CreateDepthBuffer(screenWidth, screenHeight))
		{
			return false;
		}

		return true;
	}


	bool GraphicsDevice::Present()
	{
		if (!m_swapChain)
		{
			OutputDebugStringW(L"Swap chain is Empty!");
			return false;
		}

		if (FAILED(m_swapChain->Present(1, 0)))
		{
			OutputDebugStringW(L"IDXGISwapChain::Present failed");
			return false;
		}

		return true;
	}

	bool GraphicsDevice::InitDirect3D(HWND mainWindow, int screenWidth, int screenHeight)
	{
		// m_device, m_context 생성
		UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		const D3D_FEATURE_LEVEL featureLevels[2] = { D3D_FEATURE_LEVEL_11_0 };
		D3D_FEATURE_LEVEL featureLevel;

		// swapchain 생성
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));

		sd.BufferDesc.Width = screenWidth;	  // set the back buffer width
		sd.BufferDesc.Height = screenHeight;	  // set the back buffer height
		sd.BufferDesc.RefreshRate.Numerator = 0; // use 32-bit color
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferCount = 2;	 // double buffering

		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = mainWindow; // the window to be used
		sd.Windowed = TRUE;             // windowed/full-screen mode
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // allow full-screen switching
		sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

		sd.SampleDesc.Count = 1;	// No use MSAA
		sd.SampleDesc.Quality = 0;

		if (FAILED(D3D11CreateDeviceAndSwapChain(
			0, // Default adapter
			D3D_DRIVER_TYPE_HARDWARE,
			0, // No software device
			createDeviceFlags, featureLevels, 1, D3D11_SDK_VERSION, &sd,
			m_swapChain.GetAddressOf(), m_device.GetAddressOf(), &featureLevel,
			m_context.GetAddressOf())))
		{
			OutputDebugStringW(L"D3D11CreateDeviceAndSwapChain() failed");
			return false;
		}

		if (featureLevel != D3D_FEATURE_LEVEL_11_0)
		{
			OutputDebugStringW(L"D3D Feature Level 11 unsupported");
			return false;
		}

		return true;

	}

	bool GraphicsDevice::CreateRenderTargetView()
	{
		ComPtr<ID3D11Texture2D> backBuffer;

		if (FAILED(m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()))))
		{
			OutputDebugStringW(L"GetBuffer() failed");
			return false;
		}

		if (backBuffer)
		{
			if (FAILED(m_device->CreateRenderTargetView(
				backBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf()
			)))
			{
				OutputDebugStringW(L"CreateRTV() failed");
				return false;
			}
		}
		else
		{
			OutputDebugStringW(L"BackBuffer is Empty");
			return false;
		}

		return true;
	}
	bool GraphicsDevice::CreateDepthBuffer(int screenWidth, int screenHeight)
	{
		if (!m_device)
		{
			OutputDebugStringW(L"Device is empty. must be initialized");
			return false;
		}

		if (screenWidth <= 0 || screenHeight <= 0)
		{
			OutputDebugStringW(L"Screen size must be over 0");
			return false;
		}

		D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
		ZeroMemory(&depthStencilBufferDesc, sizeof(depthStencilBufferDesc));
		depthStencilBufferDesc.Width = screenWidth;
		depthStencilBufferDesc.Height = screenHeight;
		depthStencilBufferDesc.MipLevels = 1;
		depthStencilBufferDesc.ArraySize = 1;
		depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		//if (numQualityLevels > 0) {
		//    depthStencilBufferDesc.SampleDesc.Count = 4; // how many multisamples
		//    depthStencilBufferDesc.SampleDesc.Quality = numQualityLevels - 1;
		//}
		//else {
		//    depthStencilBufferDesc.SampleDesc.Count = 1; // how many multisamples
		//    depthStencilBufferDesc.SampleDesc.Quality = 0;
		//}
		depthStencilBufferDesc.SampleDesc.Count = 1; // how many multisamples
		depthStencilBufferDesc.SampleDesc.Quality = 0;

		depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilBufferDesc.CPUAccessFlags = 0;
		depthStencilBufferDesc.MiscFlags = 0;

		if (FAILED(m_device->CreateTexture2D(&depthStencilBufferDesc, 0, m_depthTexture.GetAddressOf())))
		{
			OutputDebugStringW(L"CreateTexture2D() failed");
			return false;
		}
		if (FAILED(m_device->CreateDepthStencilView(m_depthTexture.Get(), 0, m_depthStencilView.GetAddressOf())
		))
		{
			OutputDebugStringW(L"CreateDepthStencilView() failed");
			return false;
		}

		return true;
	}
}