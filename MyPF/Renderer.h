#pragma once
#include "D3D11Utils.h"
#include "RenderItem.h"
#include "ShaderConstants.h"
#include "FrameRenderData.h"

namespace My
{
	class GraphicsDevice;

	class Renderer
	{
	public:
		bool Initialize(GraphicsDevice& graphicsDevice, int screenWidth, int screenHeight);

		bool BeginFrame(const FrameRenderData&, const std::array<float, 4>&);
		bool EndFrame();

		bool DrawRenderItem(const RenderItem&);
		bool SetSceneViewport(float topLeftX, float topLeftY, float width, float height);

	private:
		bool CreateRasterizerState();
		void SetViewPort(float topLeftX, float topLeftY, float screenWidth, float screenHeight);

	private:
		GraphicsDevice* m_graphicsDevice = nullptr;
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
		CameraConstantData m_cameraConstantData;
		ComPtr<ID3D11Buffer> m_cameraConstantBuffer;
		LightConstantData m_lightConstantData;
		ComPtr<ID3D11Buffer> m_lightConstantBuffer;
		MaterialConstantData m_materialConstantData;
		ComPtr<ID3D11Buffer> m_materialConstantBuffer;

		D3D11_VIEWPORT m_screenViewport;
	};


}


