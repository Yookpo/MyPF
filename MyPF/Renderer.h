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
		ObjectConstantData m_objectConstantData;
		ComPtr<ID3D11Buffer> m_objectConstantBuffer;
		MaterialConstantData m_materialConstantData;
		ComPtr<ID3D11Buffer> m_materialConstantBuffer;


		CameraConstantData m_cameraConstantData;
		LightConstantData m_lightConstantData;

		BufferHandle m_cameraBufferHandle;
		BufferHandle m_lightBufferHandle;



		D3D11_VIEWPORT m_screenViewport;
	};


}


