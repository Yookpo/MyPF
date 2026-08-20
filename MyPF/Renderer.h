#pragma once
#include "D3D11Utils.h"
#include "RenderItem.h"
#include "ShaderConstants.h"
#include "FrameRenderData.h"
#include "BufferHandle.h"

namespace My
{
	class GraphicsDevice;
	class GraphicsResourceManager;

	class Renderer
	{
	public:
		bool Initialize(GraphicsDevice& graphicsDevice, GraphicsResourceManager& resourceManager,
			int screenWidth, int screenHeight);

		bool BeginFrame(const FrameRenderData&, const std::array<float, 4>&);
		bool EndFrame();

		bool DrawRenderItem(const RenderItem&);
		bool SetSceneViewport(float topLeftX, float topLeftY, float width, float height);

	private:
		bool CreateRasterizerState();
		void SetViewPort(float topLeftX, float topLeftY, float screenWidth, float screenHeight);

	private:
		GraphicsDevice* m_graphicsDevice = nullptr;
		GraphicsResourceManager* m_resourceManager = nullptr;

		ComPtr<ID3D11RasterizerState> m_rasterizerState;

		// Depth buffer
		ComPtr<ID3D11DepthStencilState> m_depthStencilState;

		// shader
		ComPtr<ID3D11VertexShader> m_vertexShader;
		ComPtr<ID3D11PixelShader> m_pixelShader;
		ComPtr<ID3D11InputLayout> m_inputLayout;

		// sampler
		ComPtr<ID3D11SamplerState> m_samplerState;

		// constantData
		CameraConstantData m_cameraConstantData;
		LightConstantData m_lightConstantData;
		ObjectConstantData m_objectConstantData;
		MaterialConstantData m_materialConstantData;

		// BufferHandle -> 실제 버퍼 소유자는 리소스매니저
		BufferHandle m_cameraBufferHandle;
		BufferHandle m_lightBufferHandle;
		BufferHandle m_objectBufferHandle;
		BufferHandle m_materialBufferHandle;


		D3D11_VIEWPORT m_screenViewport;
	};


}


