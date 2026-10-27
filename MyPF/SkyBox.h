#pragma once
#include "D3D11Utils.h"
#include "BufferHandle.h"

namespace My
{
	class GraphicsResourceManager;

	class SkyBox
	{
	public:
		bool Initialize(ID3D11Device* device, GraphicsResourceManager& resourceManager,
			const vector<D3D11_INPUT_ELEMENT_DESC>& inputElements);

		bool Render(ID3D11DeviceContext* context, ID3D11Buffer* cameraBuffer, ID3D11ShaderResourceView* environmentSRV,
			ID3D11SamplerState* sampler) const;

	private:
		GraphicsResourceManager* m_resourceManager{ nullptr };

		ComPtr<ID3D11VertexShader>		m_vertexShader;
		ComPtr<ID3D11PixelShader>		m_pixelShader;
		ComPtr<ID3D11InputLayout>		m_inputLayout;
		ComPtr<ID3D11DepthStencilState> m_depthStencilState;

		// 실제 버퍼 소유자는 리소스 매니저
		BufferHandle m_vertexBufferHandle;
		BufferHandle m_indexBufferHandle;
		uint32_t	 m_indexCount{ 0 };
	};
} // namespace My
