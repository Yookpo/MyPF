#include "Renderer.h"
#include "Mesh.h"
#include "Material.h"
#include "Texture.h"
#include "GraphicsDevice.h"
#include "GraphicsResourceManager.h"

namespace My
{

	bool Renderer::Initialize(GraphicsDevice& graphicsDevice, GraphicsResourceManager& resourceManager,
		int screenWidth, int screenHeight)
	{
		m_graphicsDevice = &graphicsDevice;
		m_resourceManager = &resourceManager;

		if (!m_graphicsDevice->GetDevice() || !m_graphicsDevice->GetContext()
			|| screenWidth <= 0 || screenHeight <= 0)
		{
			return false;
		}

		ID3D11Device* Device = m_graphicsDevice->GetDevice();

		if (!CreateRasterizerState())
		{
			return false;
		}

		SetViewPort(0, 0, static_cast<float>(screenWidth), static_cast<float>(screenHeight));

		if (!D3D11Utils::CreateDepthStencilState(Device, m_depthStencilState))
		{
			return false;
		}

		// Create Buffer Handle
		m_cameraConstantData.view = Matrix();
		m_cameraConstantData.projection = Matrix();
		m_cameraBufferHandle = m_resourceManager->CreateConstantBuffer(m_cameraConstantData);

		if (!m_cameraBufferHandle.IsValid())
		{
			return false;
		}

		m_lightConstantData.direction = Vector3(0.0f, -0.5f, 1.0f);
		m_lightConstantData.color = Vector3(0.0f);
		m_lightConstantData.intensity = 1.0f;
		m_lightConstantData.pad = 0.0f;
		m_lightBufferHandle = m_resourceManager->CreateConstantBuffer(m_lightConstantData);

		if (!m_lightBufferHandle.IsValid())
		{
			return false;
		}


		m_objectConstantData.model = Matrix();
		m_objectConstantData.invTranspose = Matrix();
		m_objectBufferHandle = m_resourceManager->CreateConstantBuffer(m_objectConstantData);

		if (!m_objectBufferHandle.IsValid())
		{
			return false;
		}


		m_materialConstantData.baseColor = Vector3(1.0f);
		m_materialConstantData.pad = 0.0f;
		m_materialBufferHandle = m_resourceManager->CreateConstantBuffer(m_materialConstantData);

		if (!m_materialBufferHandle.IsValid())
		{
			return false;
		}


		vector<D3D11_INPUT_ELEMENT_DESC> inputElements = {
			{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,
			D3D11_INPUT_PER_VERTEX_DATA,0},
			{"COLOR",0,DXGI_FORMAT_R32G32B32_FLOAT,0,4 * 3,
			D3D11_INPUT_PER_VERTEX_DATA,0},
			{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,4 * 3 * 2,
			D3D11_INPUT_PER_VERTEX_DATA,0},
			{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,4 * 3 * 3,
			D3D11_INPUT_PER_VERTEX_DATA,0}
		};

		if (!D3D11Utils::CreateVertexShaderAndInputLayout(
			Device, L"Shaders\\simpleVertexShader.hlsl", inputElements, m_vertexShader,
			m_inputLayout
		))
		{
			return false;
		}

		if (!D3D11Utils::CreatePixelShader(
			Device, L"Shaders\\simplePixelShader.hlsl", m_pixelShader
		))
		{
			return false;
		}

		// Sampler 만들기
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		if (FAILED(Device->CreateSamplerState(&sampDesc, m_samplerState.GetAddressOf())))
		{
			return false;
		}

		return true;
	}


	bool Renderer::BeginFrame(const FrameRenderData& frameRenderData, const std::array<float, 4>& m_backgroundColor)
	{
		if (!m_graphicsDevice)
		{
			OutputDebugStringW(L"GraphicsDevice is Empty");
			return false;
		}

		ID3D11DeviceContext* Context = m_graphicsDevice->GetContext();
		ID3D11RenderTargetView* RTV = m_graphicsDevice->GetRTV();
		ID3D11DepthStencilView* DSV = m_graphicsDevice->GetDSV();

		if (!Context || !RTV || !DSV)
		{
			return false;
		}


		Context->ClearRenderTargetView(RTV, m_backgroundColor.data());
		Context->ClearDepthStencilView(DSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		// 비교: Depth Buffer를 사용하지 않는 경우
		// m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);
		Context->OMSetRenderTargets(1, &RTV, DSV);
		Context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

		// 카메라
		m_cameraConstantData.view = frameRenderData.view;
		m_cameraConstantData.view = m_cameraConstantData.view.Transpose();
		m_cameraConstantData.projection = frameRenderData.projection;
		m_cameraConstantData.projection = m_cameraConstantData.projection.Transpose();

		if (!m_resourceManager->UpdateBuffer(m_cameraBufferHandle, m_cameraConstantData))
		{
			return false;
		}

		// 조명
		m_lightConstantData.direction = frameRenderData.directionalLight.direction;
		m_lightConstantData.direction.Normalize();
		m_lightConstantData.color = frameRenderData.directionalLight.color;
		m_lightConstantData.intensity = frameRenderData.directionalLight.intensity;

		if (!m_resourceManager->UpdateBuffer(m_lightBufferHandle, m_lightConstantData))
		{
			return false;
		}

		ID3D11Buffer* lightconstantBuffer = m_resourceManager->GetBuffer(m_lightBufferHandle);
		if (!lightconstantBuffer)
		{
			return false;
		}

		Context->PSSetConstantBuffers(0, 1, &lightconstantBuffer);

		return true;
	}

	bool Renderer::DrawRenderItem(const RenderItem& renderItem)
	{
		if (!m_graphicsDevice)
		{
			OutputDebugStringW(L"GraphicsDevice is Empty");
			return false;
		}

		ID3D11DeviceContext* Context = m_graphicsDevice->GetContext();

		if (!Context)
		{
			OutputDebugStringW(L"Context is Empty");
			return false;
		}

		if (!renderItem.mesh || !renderItem.material)
		{
			OutputDebugStringW(L"No Data in renderItem");
			return false;
		}

		const Mesh& drawMesh = *renderItem.mesh;
		const Material& drawMat = *renderItem.material;
		const Texture* albedoTexture = drawMat.GetAlbedoTexture();

		if (!albedoTexture)
		{
			OutputDebugStringW(L"No Texture");
			return false;
		}

		// 모델 변환
		m_objectConstantData.model = renderItem.world;
		m_objectConstantData.model = m_objectConstantData.model.Transpose();

		m_objectConstantData.invTranspose = m_objectConstantData.model;
		m_objectConstantData.invTranspose.Translation(Vector3(0.0f));
		m_objectConstantData.invTranspose = m_objectConstantData.invTranspose.Transpose().Invert();

		if (!m_resourceManager->UpdateBuffer(m_objectBufferHandle, m_objectConstantData))
		{
			return false;
		}

		// 머터리얼 변환
		m_materialConstantData.baseColor = drawMat.GetBaseColor();
		if (!m_resourceManager->UpdateBuffer(m_materialBufferHandle, m_materialConstantData))
		{
			return false;
		}

		UINT stride = drawMesh.GetVertexStride();
		UINT offset = 0;

		BufferHandle vertexBufferHandle = drawMesh.GetVertexBufferHandle();
		BufferHandle indexBufferHandle = drawMesh.GetIndexBufferHandle();

		ID3D11Buffer* objectConstantBuffer = m_resourceManager->GetBuffer(m_objectBufferHandle);
		ID3D11Buffer* cameraConstantBuffer = m_resourceManager->GetBuffer(m_cameraBufferHandle);
		ID3D11Buffer* materialConstantBuffer = m_resourceManager->GetBuffer(m_materialBufferHandle);

		ID3D11Buffer* vertexBuffer = m_resourceManager->GetBuffer(vertexBufferHandle);
		ID3D11Buffer* indexBuffer = m_resourceManager->GetBuffer(indexBufferHandle);

		if (!objectConstantBuffer || !cameraConstantBuffer || !materialConstantBuffer
			|| !vertexBuffer || !indexBuffer)
		{
			return false;
		}

		ID3D11Buffer* constantBuffers[2] = {
			objectConstantBuffer,cameraConstantBuffer,
		};

		ID3D11Buffer* pixelConstantBuffers =
		{
			materialConstantBuffer
		};

		TextureHandle albedoHandle = albedoTexture->GetTextureHandle();
		if (!albedoHandle.IsValid())
		{
			return false;
		}

		ID3D11ShaderResourceView* albedoSRV = m_resourceManager->GetSRV(albedoHandle);

		if (!albedoSRV)
		{
			OutputDebugStringW(L"No SRV");
			return false;
		}

		Context->IASetInputLayout(m_inputLayout.Get());
		Context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		Context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		Context->VSSetShader(m_vertexShader.Get(), 0, 0);
		Context->VSSetConstantBuffers(0, 2, constantBuffers);
		Context->PSSetShader(m_pixelShader.Get(), 0, 0);

		Context->PSSetShaderResources(0, 1, &albedoSRV);
		Context->PSSetConstantBuffers(1, 1, &pixelConstantBuffers);
		Context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

		Context->DrawIndexed(drawMesh.GetIndexCount(), 0, 0);

		return true;
	}

	bool Renderer::SetSceneViewport(float topLeftX, float topLeftY, float width, float height)
	{
		if (!m_graphicsDevice)
		{
			OutputDebugStringW(L"GraphicsDevice is Empty");
			return false;
		}

		ID3D11DeviceContext* Context = m_graphicsDevice->GetContext();

		if (!Context)
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
		if (!m_graphicsDevice)
		{
			OutputDebugStringW(L"GraphicsDevice is Empty");
			return false;
		}

		if (!m_graphicsDevice->Present())
		{
			OutputDebugStringW(L"m_graphicsDevice::Present failed");
			return false;
		}

		return true;
	}


	void Renderer::SetViewPort(float topLeftX, float topLeftY, float screenWidth, float screenHeight)
	{
		if (!m_graphicsDevice)
		{
			OutputDebugStringW(L"GraphicsDevice is Empty");
			return;
		}

		ID3D11DeviceContext* Context = m_graphicsDevice->GetContext();

		if (!Context)
		{
			OutputDebugStringW(L"Context is Empty");
			return;
		}

		ZeroMemory(&m_screenViewport, sizeof(D3D11_VIEWPORT));
		m_screenViewport.TopLeftX = topLeftX;
		m_screenViewport.TopLeftY = topLeftY;

		m_screenViewport.Width = screenWidth;
		m_screenViewport.Height = screenHeight;
		m_screenViewport.MinDepth = 0.0f;
		m_screenViewport.MaxDepth = 1.0f;	// Note: important for depth buffering

		Context->RSSetViewports(1, &m_screenViewport);
	}


	bool Renderer::CreateRasterizerState()
	{
		if (!m_graphicsDevice)
		{
			OutputDebugStringW(L"GraphicsDevice is Empty");
			return false;
		}

		ID3D11Device* Device = m_graphicsDevice->GetDevice();
		ID3D11DeviceContext* Context = m_graphicsDevice->GetContext();

		if (!Device || !Context)
		{
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



		if (FAILED(Device->CreateRasterizerState(&rastDesc,
			m_rasterizerState.GetAddressOf())))
		{
			OutputDebugStringW(L"CreateRasterizerState() failed");
			return false;
		}

		// 초기화 후 해당 래스터 초기화 할 때 바로 적용
		Context->RSSetState(m_rasterizerState.Get());

		return true;
	}
}

