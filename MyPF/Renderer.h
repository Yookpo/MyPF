#pragma once
#include "D3D11Utils.h"
#include "RenderItem.h"
#include "ShaderConstants.h"

namespace My
{
	class Camera;

	class Renderer
	{
	public:
		bool Initialize(HWND mainWindow, int screenWidth, int screenHeight);
		bool Resize(int screenWidth, int screenHeight);

		void BeginFrame(const Camera&, const std::array<float, 4>&);
		bool EndFrame();

		bool DrawRenderItem(const RenderItem&);

		bool SetSceneViewport(float topLeftX, float topLeftY, float width, float height);

		ID3D11Device* GetDevice() const { return m_device.Get(); }
		ComPtr<ID3D11DeviceContext> GetContext() const { return m_context; }


	private:
		bool InitDirect3D(HWND mainWindow, int screenWidth, int screenHeight);
		bool CreateRenderTargetView();
		void SetViewPort(float topLeftX, float topLeftY, float screenWidth, float screenHeight);

	private:
		ComPtr<ID3D11Device> m_device;
		ComPtr<ID3D11DeviceContext> m_context;
		ComPtr<IDXGISwapChain> m_swapChain;	// 완성된 화면을 창에 표시
		ComPtr<ID3D11RenderTargetView> m_renderTargetView;	// 백 버퍼를 렌더링 대상으로 연결
		ComPtr<ID3D11RasterizerState> m_rasterizerState;

		// Depth buffer
		ComPtr<ID3D11DepthStencilView> m_depthStencilView;
		ComPtr<ID3D11DepthStencilState> m_depthStencilState;

		// shader
		ComPtr<ID3D11VertexShader> m_vertexShader;
		ComPtr<ID3D11PixelShader> m_pixelShader;
		ComPtr<ID3D11InputLayout> m_inputLayout;

		BasicVertexConstantData m_constantBufferData;
		ComPtr<ID3D11Buffer> m_constantBuffer;

		D3D11_VIEWPORT m_screenViewport;
	};


}
