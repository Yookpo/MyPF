#pragma once
#include "D3D11Utils.h"
#include "RenderItem.h"
#include "ShaderConstants.h"
#include "FrameRenderData.h"
#include "BufferHandle.h"
#include "TextureHandle.h"

namespace My
{
	class GraphicsDevice;
	class GraphicsResourceManager;

	class Renderer
	{
	public:
		bool Initialize(GraphicsDevice& graphicsDevice, GraphicsResourceManager& resourceManager, int screenWidth,
			int screenHeight);

		bool Resize(int screenWidth, int screenHeight);

		bool BeginFrame(const FrameRenderData&, const std::array<float, 4>&);
		bool EndScene();
		bool EndFrame();

		bool DrawRenderItem(const RenderItem&);
		bool SetSceneViewport(float topLeftX, float topLeftY, float width, float height);

	private:
		bool		   CreateRasterizerState();
		void		   SetViewPort(float topLeftX, float topLeftY, float screenWidth, float screenHeight);
		static Vector3 SrgbToLinear(const Vector3& sRgbcolor);
		void		   DrawFullScreenPass(ID3D11RenderTargetView* target, ID3D11PixelShader* pixelShader,
					  ID3D11ShaderResourceView* sourceSRV, ID3D11SamplerState* sampler);

	private:
		GraphicsDevice*			 m_graphicsDevice = nullptr;
		GraphicsResourceManager* m_resourceManager = nullptr;

		ComPtr<ID3D11RasterizerState> m_rasterizerState;

		// Depth buffer
		ComPtr<ID3D11DepthStencilState> m_depthStencilState;

		// shader
		ComPtr<ID3D11VertexShader> m_vertexShader;
		ComPtr<ID3D11VertexShader> m_fullscreenVertexShader;
		ComPtr<ID3D11PixelShader>  m_pixelShader;
		ComPtr<ID3D11PixelShader>  m_copyPixelShader;
		ComPtr<ID3D11PixelShader>  m_toneMappingPixelShader;
		ComPtr<ID3D11PixelShader>  m_brightPassPixelShader;
		ComPtr<ID3D11PixelShader>  m_blurXPixelShader;
		ComPtr<ID3D11PixelShader>  m_blurYPixelShader;
		ComPtr<ID3D11InputLayout>  m_inputLayout;

		// sampler
		ComPtr<ID3D11SamplerState> m_samplerState;
		ComPtr<ID3D11SamplerState> m_clampSamplerState;

		// constantData
		CameraConstantData		m_cameraConstantData{};
		LightConstantData		m_lightConstantData{};
		ObjectConstantData		m_objectConstantData{};
		MaterialConstantData	m_materialConstantData{};
		PostProcessConstantData m_postProcessConstantData{};

		// BufferHandle -> 실제 버퍼 소유자는 리소스매니저
		BufferHandle m_cameraBufferHandle;
		BufferHandle m_lightBufferHandle;
		BufferHandle m_objectBufferHandle;
		BufferHandle m_materialBufferHandle;
		BufferHandle m_postProcessBufferHandle;

		// for Bloom
		TextureHandle m_hdrSceneTargetHandle;
		TextureHandle m_bloomBrightTargetHandle;
		TextureHandle m_bloomBlurXTargetHandle;
		TextureHandle m_bloomBlurYTargetHandle;

		D3D11_VIEWPORT m_screenViewport;

		uint32_t m_bloomWidth{ 0 };
		uint32_t m_bloomHeight{ 0 };

		PostProcessDebugView m_debugView{ PostProcessDebugView::Final };
	};

} // namespace My
