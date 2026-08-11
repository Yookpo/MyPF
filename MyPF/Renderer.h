#pragma once
#include "D3D11Utils.h"
#include "GeometryGenerator.h"

namespace My
{
	class Renderer
	{
	public:
		bool Initialize(HWND mainWindow, int screenWidth, int screenHeight);
		bool Resize(int screenWidth, int screenHeight);

		void BeginFrame(const std::array<float, 4>& m_backgroundColor);
		bool EndFrame();

		bool DrawCube(const Matrix&);

		bool SetSceneViewport(float topLeftX, float topLeftY, float width, float height);

		ComPtr<ID3D11Device> GetDevice() const { return m_device; }
		ComPtr<ID3D11DeviceContext> GetContext() const { return m_context; }


	private:
		bool InitDirect3D(HWND mainWindow, int screenWidth, int screenHeight);
		bool CreateRenderTargetView();
		void SetViewPort(float topLeftX, float topLeftY, float screenWidth, float screenHeight);
		float GetAspectRatio(float screenWidth, float screenHeight) const;

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

		// meshes
		ComPtr<ID3D11Buffer> m_vertexBuffer;
		ComPtr<ID3D11Buffer> m_indexBuffer;
		UINT m_indexCount = 0;

		BasicVertexConstantData m_constantBufferData;
		ComPtr<ID3D11Buffer> m_constantBuffer;

		// Data
		bool m_usePerspectiveProjection = true;
		Vector3 m_viewEyePos = { 0.0f, 0.0f, -2.0f };
		Vector3 m_viewEyeDir = { 0.0f, 0.0f, 1.0f };
		Vector3 m_viewUp = { 0.0f, 1.0f, 0.0f };
		float m_projFovAngleY = 70.0f;
		float m_nearZ = 0.01f;
		float m_farZ = 100.0f;
		float m_aspect;

		D3D11_VIEWPORT m_screenViewport;
	};


}
