#include "Renderer.h"
#include "Mesh.h"
#include "Camera.h"

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

		SetViewPort(0, 0, static_cast<float>(screenWidth), static_cast<float>(screenHeight));


		m_constantBufferData.model = Matrix();
		m_constantBufferData.view = Matrix();
		m_constantBufferData.projection = Matrix();

		if (!D3D11Utils::CreateConstantBuffer(m_device, m_constantBufferData, m_constantBuffer))
		{
			return false;
		}

		if (!D3D11Utils::CreateDepthBuffer(m_device, screenWidth, screenHeight, m_depthStencilView, m_depthStencilState))
		{
			return false;
		}

		vector<D3D11_INPUT_ELEMENT_DESC> inputElements = {
			{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,
			D3D11_INPUT_PER_VERTEX_DATA,0},
			{"COLOR",0,DXGI_FORMAT_R32G32B32_FLOAT,0,4 * 3,
			D3D11_INPUT_PER_VERTEX_DATA,0}
		};

		if (!D3D11Utils::CreateVertexShaderAndInputLayout(
			m_device, L"Shaders\\simpleVertexShader.hlsl", inputElements, m_vertexShader,
			m_inputLayout
		))
		{
			return false;
		}

		if (!D3D11Utils::CreatePixelShader(
			m_device, L"Shaders\\simplePixelShader.hlsl", m_pixelShader
		))
		{
			return false;
		}

		return true;
	}

	bool Renderer::Resize(int screenWidth, int screenHeight)
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
		m_depthStencilState.Reset();

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
		if (!D3D11Utils::CreateDepthBuffer(m_device, screenWidth, screenHeight,
			m_depthStencilView, m_depthStencilState))
		{
			return false;
		}

		return true;
	}

	void Renderer::BeginFrame(const Camera& m_camera, const std::array<float, 4>& m_backgroundColor)
	{
		m_context->ClearRenderTargetView(m_renderTargetView.Get(), m_backgroundColor.data());
		m_context->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		// 비교: Depth Buffer를 사용하지 않는 경우
		// m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);
		m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
		m_context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

		// 시점 변환
		m_constantBufferData.view = m_camera.GetViewMatrix();
		m_constantBufferData.view = m_constantBufferData.view.Transpose();

		// 프로젝션
		m_constantBufferData.projection = m_camera.GetProjectionMatrix();
		m_constantBufferData.projection = m_constantBufferData.projection.Transpose();
	}

	bool Renderer::DrawRenderItem(const RenderItem& renderItem)
	{
		if (!renderItem.mesh)
		{
			OutputDebugStringW(L"No Mesh in renderItem");
			return false;
		}

		const Mesh& drawMesh = *renderItem.mesh;

		// 모델 변환
		m_constantBufferData.model = renderItem.world;
		m_constantBufferData.model = m_constantBufferData.model.Transpose();

		if (!D3D11Utils::UpdateBuffer(m_context, m_constantBufferData, m_constantBuffer))
		{
			return false;
		}

		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		ID3D11Buffer* meshVertexBuffer = drawMesh.GetVertexBuffer();

		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetVertexBuffers(0, 1, &meshVertexBuffer, &stride, &offset);
		m_context->IASetIndexBuffer(drawMesh.GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		m_context->VSSetShader(m_vertexShader.Get(), 0, 0);
		m_context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
		m_context->PSSetShader(m_pixelShader.Get(), 0, 0);

		m_context->DrawIndexed(drawMesh.GetIndexCount(), 0, 0);

		return true;
	}

	bool Renderer::SetSceneViewport(float topLeftX, float topLeftY, float width, float height)
	{
		if (!m_context)
		{
			OutputDebugStringW(L"Context is Empty");
			return false;
		}
		if (width <= 0 || height <= 0)
		{
			return false;
		}

		this->SetViewPort(topLeftX, topLeftY, width, height);

		return true;
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
			OutputDebugStringW(L"CreateRasterizerState() failed");
			return false;
		}

		// 초기화 후 해당 래스터 초기화 할 때 바로 적용
		m_context->RSSetState(m_rasterizerState.Get());

		return true;
	}

	void Renderer::SetViewPort(float topLeftX, float topLeftY, float screenWidth, float screenHeight)
	{
		ZeroMemory(&m_screenViewport, sizeof(D3D11_VIEWPORT));
		m_screenViewport.TopLeftX = topLeftX;
		m_screenViewport.TopLeftY = topLeftY;

		m_screenViewport.Width = screenWidth;
		m_screenViewport.Height = screenHeight;
		m_screenViewport.MinDepth = 0.0f;
		m_screenViewport.MaxDepth = 1.0f;	// Note: important for depth buffering

		m_context->RSSetViewports(1, &m_screenViewport);
	}

	bool Renderer::CreateRenderTargetView()
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
}

