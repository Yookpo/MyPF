#include <cmath>
#include <algorithm>
#include "Renderer.h"
#include "Mesh.h"
#include "Material.h"
#include "Texture.h"
#include "GraphicsDevice.h"
#include "GraphicsResourceManager.h"

namespace My
{

	bool Renderer::Initialize(
		GraphicsDevice& graphicsDevice, GraphicsResourceManager& resourceManager, int screenWidth, int screenHeight)
	{
		m_graphicsDevice = &graphicsDevice;
		m_resourceManager = &resourceManager;

		if (!m_graphicsDevice->GetDevice() || !m_graphicsDevice->GetContext() || screenWidth <= 0 || screenHeight <= 0)
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
		m_cameraConstantData.cameraPosition = Vector3(0.0f);
		m_cameraBufferHandle = m_resourceManager->CreateConstantBuffer(m_cameraConstantData);

		if (!m_cameraBufferHandle.IsValid())
		{
			return false;
		}

		m_lightConstantData.direction = Vector3(0.0f, -0.5f, 1.0f);
		m_lightConstantData.color = Vector3(0.0f);
		m_lightConstantData.intensity = 1.0f;
		m_lightConstantData.ambientStrength = 0.4f;

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
		m_materialConstantData.emissiveColor = Vector3(1.0f);
		m_materialConstantData.emissiveIntensity = 0.0f;
		m_materialConstantData.rimColor = Vector3(1.0f);
		m_materialConstantData.rimIntensity = 0.0f;
		m_materialConstantData.rimPower = 3.0f;
		m_materialBufferHandle = m_resourceManager->CreateConstantBuffer(m_materialConstantData);

		if (!m_materialBufferHandle.IsValid())
		{
			return false;
		}

		m_postProcessConstantData.exposure = 1.0f;
		m_postProcessBufferHandle = m_resourceManager->CreateConstantBuffer(m_postProcessConstantData);

		if (!m_postProcessBufferHandle.IsValid())
		{
			return false;
		}

		m_hdrSceneTargetHandle = m_resourceManager->CreateRenderTarget(
			static_cast<uint32_t>(screenWidth), static_cast<uint32_t>(screenHeight), DXGI_FORMAT_R16G16B16A16_FLOAT);

		if (!m_hdrSceneTargetHandle.IsValid())
		{
			return false;
		}

		m_bloomWidth = static_cast<uint32_t>((std::max)(1, screenWidth / 2));
		m_bloomHeight = static_cast<uint32_t>((std::max)(1, screenHeight / 2));

		m_bloomBrightTargetHandle =
			m_resourceManager->CreateRenderTarget(m_bloomWidth, m_bloomHeight, DXGI_FORMAT_R16G16B16A16_FLOAT);

		if (!m_bloomBrightTargetHandle.IsValid())
		{
			return false;
		}

		// debug
		std::wstring msg = L"HDR Scene Target created: " + std::to_wstring(screenWidth) + L" x "
			+ std::to_wstring(screenHeight) + L", index " + std::to_wstring(m_hdrSceneTargetHandle.GetIndex()) + L"\n";
		OutputDebugStringW(msg.c_str());

		// debug
		std::wstring msg1 = L"Bloom Bright Target created: " + std::to_wstring(m_bloomWidth) + L" x "
			+ std::to_wstring(m_bloomHeight) + L", index " + std::to_wstring(m_bloomBrightTargetHandle.GetIndex())
			+ L"\n";
		OutputDebugStringW(msg1.c_str());

		vector<D3D11_INPUT_ELEMENT_DESC> inputElements = { { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
															   D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3 * 2, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 3 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 } };

		if (!D3D11Utils::CreateVertexShaderAndInputLayout(
				Device, L"Shaders\\simpleVertexShader.hlsl", inputElements, m_vertexShader, m_inputLayout))
		{
			return false;
		}

		if (!D3D11Utils::CreatePixelShader(Device, L"Shaders\\simplePixelShader.hlsl", m_pixelShader))
		{
			return false;
		}

		if (!D3D11Utils::CreateVertexShader(Device, L"Shaders\\fullscreenVertexShader.hlsl", m_fullscreenVertexShader))
		{
			OutputDebugStringW(L"fullscreenVertexShader Created Failed\n");
			return false;
		}

		if (!D3D11Utils::CreatePixelShader(Device, L"Shaders\\copyPixelShader.hlsl", m_copyPixelShader))
		{
			OutputDebugStringW(L"copyPixelShader Created Failed\n");
			return false;
		}

		if (!D3D11Utils::CreatePixelShader(Device, L"Shaders\\toneMappingPixelShader.hlsl", m_toneMappingPixelShader))
		{
			OutputDebugStringW(L"toneMappingPixelShader Created Failed\n");
			return false;
		}

		if (!D3D11Utils::CreatePixelShader(Device, L"Shaders\\brightPassPixelShader.hlsl", m_brightPassPixelShader))
		{
			OutputDebugStringW(L"brightPassPixelShader Created Failed\n");
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

	bool Renderer::Resize(int screenWidth, int screenHeight)
	{
		if (screenWidth <= 0 || screenHeight <= 0)
		{
			return true;
		}

		if (!m_resourceManager || !m_hdrSceneTargetHandle.IsValid() || !m_bloomBrightTargetHandle.IsValid())
		{
			return true;
		}

		if (!m_resourceManager->ResizeRenderTarget(
				m_hdrSceneTargetHandle, static_cast<uint32_t>(screenWidth), static_cast<uint32_t>(screenHeight)))
		{
			OutputDebugStringW(L"ResizeRenderTarget() Failed\n");
			return false;
		}

		m_bloomWidth = static_cast<uint32_t>((std::max)(1, screenWidth / 2));
		m_bloomHeight = static_cast<uint32_t>((std::max)(1, screenHeight / 2));

		if (!m_resourceManager->ResizeRenderTarget(m_bloomBrightTargetHandle, m_bloomWidth, m_bloomHeight))
		{
			OutputDebugStringW(L"Bloom Bright Target resize failed\n");
			return false;
		}

		// debug
		std::wstring msg = L"HDR Scene Target resized: " + std::to_wstring(screenWidth) + L" x "
			+ std::to_wstring(screenHeight) + L", index " + std::to_wstring(m_hdrSceneTargetHandle.GetIndex()) + L"\n";
		OutputDebugStringW(msg.c_str());

		std::wstring msg1 = L"Bloom Bright Target resized: " + std::to_wstring(m_bloomWidth) + L" x "
			+ std::to_wstring(m_bloomHeight) + L", index " + std::to_wstring(m_bloomBrightTargetHandle.GetIndex())
			+ L"\n";
		OutputDebugStringW(msg1.c_str());

		return true;
	}

	bool Renderer::BeginFrame(const FrameRenderData& frameRenderData, const std::array<float, 4>& m_backgroundColor)
	{
		if (!m_graphicsDevice)
		{
			OutputDebugStringW(L"GraphicsDevice is Empty\n");
			return false;
		}

		if (!m_resourceManager)
		{
			OutputDebugStringW(L"ResourceManager is Empty\n");
			return false;
		}

		ID3D11DeviceContext* Context = m_graphicsDevice->GetContext();
		// ID3D11RenderTargetView* RTV = m_graphicsDevice->GetRTV(); // 백버퍼에서 가져온다
		ID3D11RenderTargetView* sceneRTV = m_resourceManager->GetRTV(m_hdrSceneTargetHandle);
		ID3D11DepthStencilView* DSV = m_graphicsDevice->GetDSV();

		if (!Context || !sceneRTV || !DSV)
		{
			return false;
		}

		Vector3 backgroundLinear =
			SrgbToLinear(Vector3(m_backgroundColor[0], m_backgroundColor[1], m_backgroundColor[2]));
		const std::array<float, 4> clearColor = { backgroundLinear.x, backgroundLinear.y, backgroundLinear.z,
			m_backgroundColor[3] };

		Context->ClearRenderTargetView(sceneRTV, clearColor.data());
		Context->ClearDepthStencilView(DSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		// 비교: Depth Buffer를 사용하지 않는 경우
		// m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);
		Context->OMSetRenderTargets(1, &sceneRTV, DSV);
		Context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

		// 카메라
		m_cameraConstantData.view = frameRenderData.view;
		m_cameraConstantData.view = m_cameraConstantData.view.Transpose();
		m_cameraConstantData.projection = frameRenderData.projection;
		m_cameraConstantData.projection = m_cameraConstantData.projection.Transpose();
		m_cameraConstantData.cameraPosition = frameRenderData.cameraPosition;

		if (!m_resourceManager->UpdateBuffer(m_cameraBufferHandle, m_cameraConstantData))
		{
			return false;
		}

		// 조명 처리
		// Directional Light
		m_lightConstantData.direction = frameRenderData.directionalLight.direction;
		m_lightConstantData.direction.Normalize();
		m_lightConstantData.color = SrgbToLinear(frameRenderData.directionalLight.color);
		m_lightConstantData.intensity = frameRenderData.directionalLight.intensity;
		m_lightConstantData.ambientStrength = frameRenderData.directionalLight.ambientStrength;
		// Point Light
		m_lightConstantData.pointLightCount = static_cast<std::uint32_t>(frameRenderData.pointLightCount);
		for (std::size_t i = 0; i < frameRenderData.pointLightCount; i++)
		{
			const auto&				sourceLight = frameRenderData.pointLights[i];
			PointLightConstantData& destinationLight = m_lightConstantData.pointLights[i];

			destinationLight.position = sourceLight.position;
			destinationLight.range = sourceLight.range;
			destinationLight.color = SrgbToLinear(sourceLight.color);
			destinationLight.intensity = sourceLight.intensity;
			destinationLight.isEnabled = sourceLight.isEnabled ? 1u : 0u;
		}

		if (!m_resourceManager->UpdateBuffer(m_lightBufferHandle, m_lightConstantData))
		{
			return false;
		}

		m_postProcessConstantData.exposure = frameRenderData.postProcess.exposure;
		m_postProcessConstantData.toneMapper = static_cast<std::uint32_t>(frameRenderData.postProcess.toneMapper);
		m_postProcessConstantData.threshold = frameRenderData.postProcess.threshold;

		if (!m_resourceManager->UpdateBuffer(m_postProcessBufferHandle, m_postProcessConstantData))
		{
			return false;
		}

		m_debugView = frameRenderData.postProcess.debugView;

		ID3D11Buffer* lightconstantBuffer = m_resourceManager->GetBuffer(m_lightBufferHandle);
		if (!lightconstantBuffer)
		{
			return false;
		}

		Context->PSSetConstantBuffers(0, 1, &lightconstantBuffer);

		return true;
	}

	bool Renderer::EndScene()
	{
		if (!m_graphicsDevice)
		{
			OutputDebugStringW(L"m_graphicsDevice is NULL\n");
			return false;
		}

		if (!m_graphicsDevice->GetContext())
		{
			OutputDebugStringW(L"Context is NULL\n");
			return false;
		}

		if (!m_graphicsDevice->GetRTV())
		{
			OutputDebugStringW(L"BackBuffer is NULL\n");
			return false;
		}

		if (!m_resourceManager->GetSRV(m_hdrSceneTargetHandle))
		{
			OutputDebugStringW(L"HDR SRV is NULL\n");
			return false;
		}

		if (!m_resourceManager->GetSRV(m_bloomBrightTargetHandle))
		{
			OutputDebugStringW(L"Bloom SRV is NULL\n");
			return false;
		}

		if (!m_resourceManager->GetRTV(m_bloomBrightTargetHandle))
		{
			OutputDebugStringW(L"Bloom RTV is NULL\n");
			return false;
		}

		if (!m_resourceManager->GetBuffer(m_postProcessBufferHandle))
		{
			OutputDebugStringW(L"PostProcessBuffer is NULL\n");
			return false;
		}

		ID3D11Buffer*			  postProcessConstantBuffer = m_resourceManager->GetBuffer(m_postProcessBufferHandle);
		ID3D11DeviceContext*	  context = m_graphicsDevice->GetContext();
		ID3D11RenderTargetView*	  backRTV = m_graphicsDevice->GetRTV();
		ID3D11RenderTargetView*	  bloomRTV = m_resourceManager->GetRTV(m_bloomBrightTargetHandle);
		ID3D11ShaderResourceView* bloomSRV = m_resourceManager->GetSRV(m_bloomBrightTargetHandle);
		ID3D11ShaderResourceView* hdrSRV = m_resourceManager->GetSRV(m_hdrSceneTargetHandle);
		ID3D11ShaderResourceView* nullSRV = nullptr;

		// 추출 패스
		context->OMSetRenderTargets(1, &bloomRTV, nullptr);

		// 블룸전용 뷰포트
		D3D11_VIEWPORT bloomViewPort{};
		bloomViewPort.TopLeftX = bloomViewPort.TopLeftY = 0;
		bloomViewPort.Width = static_cast<FLOAT>(m_bloomWidth);
		bloomViewPort.Height = static_cast<FLOAT>(m_bloomHeight);
		bloomViewPort.MinDepth = 0;
		bloomViewPort.MaxDepth = 1;

		context->RSSetViewports(1, &bloomViewPort);
		context->PSSetShader(m_brightPassPixelShader.Get(), 0, 0);

		context->IASetInputLayout(nullptr);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->VSSetShader(m_fullscreenVertexShader.Get(), 0, 0);
		context->PSSetShaderResources(0, 1, &hdrSRV);
		context->PSSetConstantBuffers(0, 1, &postProcessConstantBuffer);
		context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
		// 그리기
		context->Draw(3, 0);

		// 입력 해제
		context->PSSetShaderResources(0, 1, &nullSRV);

		// 뷰포트 복구
		context->RSSetViewports(1, &m_screenViewport);

		// 패스 시작 전 : 무엇을 쓸 지 선택.
		ID3D11PixelShader*		  finalPS = m_toneMappingPixelShader.Get();
		ID3D11ShaderResourceView* finalSRV = hdrSRV;
		if (m_debugView == PostProcessDebugView::Bright)
		{
			finalPS = m_copyPixelShader.Get();
			finalSRV = bloomSRV;
		}

		// 톤 매핑 패스
		// HDR RTV를 출력에서 제거
		context->OMSetRenderTargets(1, &backRTV, nullptr);

		// 정점 데이터를 쓰지않음
		context->IASetInputLayout(nullptr);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// 쉐이더 설정
		context->VSSetShader(m_fullscreenVertexShader.Get(), 0, 0);
		context->PSSetShader(finalPS, 0, 0);

		// 입력 바인딩
		context->PSSetShaderResources(0, 1, &finalSRV);
		context->PSSetConstantBuffers(0, 1, &postProcessConstantBuffer);
		context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

		// 그리기
		context->Draw(3, 0);

		// 입력 해제
		context->PSSetShaderResources(0, 1, &nullSRV);

		return true;
	}

	bool Renderer::EndFrame()
	{
		if (!m_graphicsDevice)
		{
			OutputDebugStringW(L"GraphicsDevice is Empty\n");
			return false;
		}

		if (!m_graphicsDevice->Present())
		{
			OutputDebugStringW(L"m_graphicsDevice::Present failed\n");
			return false;
		}

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

		const Mesh&		drawMesh = *renderItem.mesh;
		const Material& drawMat = *renderItem.material;
		const Texture*	albedoTexture = drawMat.GetAlbedoTexture();

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
		m_materialConstantData.baseColor = SrgbToLinear(drawMat.GetBaseColor());
		m_materialConstantData.emissiveColor = SrgbToLinear(drawMat.GetEmissiveColor());
		m_materialConstantData.emissiveIntensity = drawMat.GetEffectiveEmissiveIntensity();
		m_materialConstantData.rimColor = SrgbToLinear(drawMat.GetRimColor());
		m_materialConstantData.rimIntensity = drawMat.GetRimIntensity();
		m_materialConstantData.rimPower = drawMat.GetRimPower();
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

		if (!objectConstantBuffer || !cameraConstantBuffer || !materialConstantBuffer || !vertexBuffer || !indexBuffer)
		{
			return false;
		}

		ID3D11Buffer* constantBuffers[2] = {
			objectConstantBuffer,
			cameraConstantBuffer,
		};

		ID3D11Buffer* pixelConstantBuffers[2] = { materialConstantBuffer, cameraConstantBuffer };

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
		Context->PSSetConstantBuffers(1, 2, pixelConstantBuffers); // PS의 b0=Light, b1=Material, b2=Camera
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
		m_screenViewport.MaxDepth = 1.0f; // Note: important for depth buffering

		Context->RSSetViewports(1, &m_screenViewport);
	}

	Vector3 Renderer::SrgbToLinear(const Vector3& sRgbcolor)
	{
		Vector3 linearColor{ 0.0f };

		linearColor.x = std::pow(sRgbcolor.x, 2.2f);
		linearColor.y = std::pow(sRgbcolor.y, 2.2f);
		linearColor.z = std::pow(sRgbcolor.z, 2.2f);

		return linearColor;
	}

	bool Renderer::CreateRasterizerState()
	{
		if (!m_graphicsDevice)
		{
			OutputDebugStringW(L"GraphicsDevice is Empty");
			return false;
		}

		ID3D11Device*		 Device = m_graphicsDevice->GetDevice();
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

		if (FAILED(Device->CreateRasterizerState(&rastDesc, m_rasterizerState.GetAddressOf())))
		{
			OutputDebugStringW(L"CreateRasterizerState() failed");
			return false;
		}

		// 초기화 후 해당 래스터 초기화 할 때 바로 적용
		Context->RSSetState(m_rasterizerState.Get());

		return true;
	}
} // namespace My
