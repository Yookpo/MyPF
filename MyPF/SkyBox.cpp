#include "SkyBox.h"
#include "GraphicsResourceManager.h"
#include "GeometryGenerator.h"

namespace My
{
	bool SkyBox::Initialize(ID3D11Device* device, GraphicsResourceManager& resourceManager,
		const vector<D3D11_INPUT_ELEMENT_DESC>& inputElements)
	{
		m_resourceManager = &resourceManager;

		if (!D3D11Utils::CreateVertexShaderAndInputLayout(
				device, L"Shaders\\skyboxVertexShader.hlsl", inputElements, m_vertexShader, m_inputLayout))
		{
			OutputDebugStringW(L"skyboxVertexShader Created Failed\n");
			return false;
		}

		if (!D3D11Utils::CreatePixelShader(device, L"Shaders\\skyboxPixelShader.hlsl", m_pixelShader))
		{
			OutputDebugStringW(L"skyboxPixelShader Created Failed\n");
			return false;
		}

		// 스카이박스 깊이 : 하늘은 깊이를 쓰지 않는다
		D3D11_DEPTH_STENCIL_DESC depthDesc{};
		depthDesc.DepthEnable = TRUE;
		depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		if (FAILED(device->CreateDepthStencilState(&depthDesc, m_depthStencilState.GetAddressOf())))
		{
			return false;
		}

		// 스카이박스 큐브 : 방향만 쓰기 때문에 크기 상관없음
		MeshData cube = GeometryGenerator::MakeCube();
		m_vertexBufferHandle = m_resourceManager->CreateVertexBuffer(cube.vertices);
		m_indexBufferHandle = m_resourceManager->CreateIndexBuffer(cube.indices);
		m_indexCount = static_cast<uint32_t>(cube.indices.size());

		return (m_vertexBufferHandle.IsValid() && m_indexBufferHandle.IsValid());
	}
	bool SkyBox::Render(ID3D11DeviceContext* context, ID3D11Buffer* cameraBuffer,
		ID3D11ShaderResourceView* environmentSRV, ID3D11SamplerState* sampler) const
	{
		ID3D11Buffer* vertexBuffer = m_resourceManager->GetBuffer(m_vertexBufferHandle);
		ID3D11Buffer* indexBuffer = m_resourceManager->GetBuffer(m_indexBufferHandle);

		if (!context || !vertexBuffer || !indexBuffer || !cameraBuffer || !environmentSRV || !sampler)
		{
			OutputDebugStringW(L"Skybox resource is NULL\n");
			return false;
		}

		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		context->IASetInputLayout(m_inputLayout.Get());
		context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		context->VSSetShader(m_vertexShader.Get(), 0, 0);
		context->VSSetConstantBuffers(1, 1, &cameraBuffer); // VS b1 = Camera
		context->PSSetShader(m_pixelShader.Get(), 0, 0);
		context->PSSetShaderResources(0, 1, &environmentSRV);
		context->PSSetSamplers(0, 1, &sampler);
		context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

		context->DrawIndexed(m_indexCount, 0, 0);

		// 입력 해제
		ID3D11ShaderResourceView* nullSRV = nullptr;
		context->PSSetShaderResources(0, 1, &nullSRV);

		return true;
	}
} // namespace My