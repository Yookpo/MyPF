#pragma once
#include <iostream>
#include <vector>
#include <windows.h>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl.h>

namespace My
{
	using Microsoft::WRL::ComPtr;
	using std::vector;
	using std::wstring;


	class AppBase
	{
	public:
		AppBase();
		virtual ~AppBase();

		int Run();

		virtual bool Initialize();
		virtual void Update(float dt);
		virtual void Render();

		virtual LRESULT MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


	protected:
		bool InitMainWindow();
		bool InitDirect3D();

		void SetViewPort();
		bool CreateRenderTargetView();

	public:
		int m_screenWidth;	// 렌더링할 최종 화면의 해상도
		int m_screenHeight;
		HWND m_mainWindow;
		UINT numQualityLevels;

		ComPtr<ID3D11Device> m_device;
		ComPtr<ID3D11DeviceContext> m_context;
		ComPtr<IDXGISwapChain> m_swapChain;	// 완성된 화면을 창에 표시
		ComPtr<ID3D11RenderTargetView> m_renderTargetView;	// 백 버퍼를 렌더링 대상으로 연결

		ComPtr<ID3D11RasterizerState> m_rasterizerState;

		D3D11_VIEWPORT m_screenViewport;
	};

}

