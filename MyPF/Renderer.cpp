#include "Renderer.h"
#include "GeometryGenerator.h"

namespace My
{
	bool Renderer::Initialize(HWND mainWindow, int screenWidth, int screenHeight)
	{
		if (!InitDirect3D(mainWindow, screenWidth, screenHeight))
		{
			return false;
		}

		if (!CreateRenderTargetView())
			return false;

		SetViewPort(screenWidth, screenHeight);

		MeshData triangle = GeometryGenerator::MakeTriangle();

		D3D11Utils::CreateVertexBuffer(m_device, triangle.vertices,
			m_vertexBuffer);
		m_indexCount = UINT(triangle.indices.size());
		D3D11Utils::CreateIndexBuffer(m_device, triangle.indices, m_indexBuffer);

		vector<D3D11_INPUT_ELEMENT_DESC> inputElements = {
			{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,
			D3D11_INPUT_PER_VERTEX_DATA,0},
			{"COLOR",0,DXGI_FORMAT_R32G32B32_FLOAT,0,4 * 3,
			D3D11_INPUT_PER_VERTEX_DATA,0}
		};

		D3D11Utils::CreateVertexShaderAndInputLayout(
			m_device, L"Shaders\\simpleVertexShader.hlsl", inputElements, m_vertexShader,
			m_inputLayout
		);

		D3D11Utils::CreatePixelShader(
			m_device, L"Shaders\\simplePixelShader.hlsl", m_pixelShader
		);

		return true;
	}

	void Renderer::BeginFrame(const std::array<float, 4>& m_backgroundColor)
	{
		m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);
		m_context->ClearRenderTargetView(m_renderTargetView.Get(), m_backgroundColor.data());
	}

	void Renderer::DrawTriangle()
	{
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
		m_context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		m_context->VSSetShader(m_vertexShader.Get(), 0, 0);
		m_context->PSSetShader(m_pixelShader.Get(), 0, 0);

		m_context->DrawIndexed(m_indexCount, 0, 0);
	}

	bool Renderer::EndFrame()
	{
		if (FAILED(m_swapChain->Present(1, 0)))
		{
			OutputDebugStringW(L"IDXGISwapChain::Present failed");
			return false;
		}

		return true;
	}




	bool Renderer::InitDirect3D(HWND mainWindow, int screenWidth, int screenHeight)
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
			m_context.GetAddressOf()))) {
			std::cerr << "D3D11CreateDeviceAndSwapChain() failed." << std::endl;
			return false;
		}

		if (featureLevel != D3D_FEATURE_LEVEL_11_0) {
			std::cerr << "D3D Feature Level 11 unsupported." << std::endl;
			return false;
		}



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

	void Renderer::SetViewPort(int screenWidth, int screenHeight)
	{
		ZeroMemory(&m_screenViewport, sizeof(D3D11_VIEWPORT));
		m_screenViewport.TopLeftX = 0;
		m_screenViewport.TopLeftY = 0;

		m_screenViewport.Width = static_cast<float>(screenWidth);
		m_screenViewport.Height = static_cast<float>(screenHeight);
		m_screenViewport.MinDepth = 0.0f;
		m_screenViewport.MaxDepth = 1.0f;	// Note: important for depth buffering

		m_context->RSSetViewports(1, &m_screenViewport);
	}

	bool Renderer::CreateRenderTargetView()
	{
		ComPtr<ID3D11Texture2D> backBuffer;

		if (FAILED(m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()))))
		{
			return false;
		}

		if (backBuffer)
		{
			if (FAILED(m_device->CreateRenderTargetView(
				backBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf()
			)))
			{
				return false;
			}
		}
		else
		{
			std::cerr << "CreateRenderTargetView() failed\n";
			return false;
		}

		return true;
	}
}