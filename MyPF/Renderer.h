#pragma once
#include "D3D11Utils.h"


namespace My
{
	class Renderer
	{
	public:
		bool Initialize(HWND mainWindow, int screenWidth, int screenHeight);

		void BeginFrame(const std::array<float, 4>& color);

		bool EndFrame();

		void DrawTriangle();

		ComPtr<ID3D11Device> GetDevice() const { return m_device; }
		ComPtr<ID3D11DeviceContext> GetContext() const { return m_context; }

	private:
		bool InitDirect3D(HWND mainWindow, int screenWidth, int screenHeight);
		void SetViewPort(int screenWidth, int screenHeight);
		bool CreateRenderTargetView();


	private:
		ComPtr<ID3D11Device> m_device;
		ComPtr<ID3D11DeviceContext> m_context;
		ComPtr<IDXGISwapChain> m_swapChain;	// 완성된 화면을 창에 표시
		ComPtr<ID3D11RenderTargetView> m_renderTargetView;	// 백 버퍼를 렌더링 대상으로 연결
		ComPtr<ID3D11RasterizerState> m_rasterizerState;

		//shader
		ComPtr<ID3D11VertexShader> m_vertexShader;
		ComPtr<ID3D11PixelShader> m_pixelShader;
		ComPtr<ID3D11InputLayout> m_inputLayout;

		//meshes
		ComPtr<ID3D11Buffer> m_vertexBuffer;
		ComPtr<ID3D11Buffer> m_indexBuffer;
		UINT m_indexCount = 0;


		D3D11_VIEWPORT m_screenViewport;
	};


}
