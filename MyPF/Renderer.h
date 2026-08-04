#pragma once
#include <iostream>
#include <vector>
#include <array>
#include <memory>
#include <windows.h>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl.h>


namespace My
{
	using Microsoft::WRL::ComPtr;
	using std::vector;
	using std::wstring;

	class Renderer
	{
	public:
		bool Initialize(HWND mainWindow, int screenWidth, int screenHeight);

		void BeginFrame(const std::array<float, 4>& color);

		bool EndFrame();

	private:
		bool InitDirect3D(HWND mainWindow, int screenWidth, int screenHeight);
		void SetViewPort(int screenWidth, int screenHeight);
		bool CreateRenderTargetView();


	private:
		ComPtr<ID3D11Device> m_device;
		ComPtr<ID3D11DeviceContext> m_context;
		ComPtr<IDXGISwapChain> m_swapChain;	// 완성된 화면을 창에 표시
		ComPtr<ID3D11RenderTargetView> m_renderTargetView;	// 백 버퍼를 렌더링 대상으로 연결
		//ComPtr<ID3D11RasterizerState> m_rasterizerState;

		D3D11_VIEWPORT m_screenViewport;
	};


}
