#pragma once
#include <d3d11.h>
#include <windows.h>
#include <wrl.h> // comptr

namespace My
{
	using Microsoft::WRL::ComPtr;

	class GraphicsDevice
	{
	public:
		GraphicsDevice() {}
		GraphicsDevice(const GraphicsDevice&) = delete;
		GraphicsDevice& operator=(const GraphicsDevice&) = delete;

		bool Initialize(HWND mainWindow, int screenWidth, int screenHeight);
		bool Resize(int screenWidth, int screenHeight);
		bool Present();

		ID3D11Device*			GetDevice() const { return m_device.Get(); }
		ID3D11DeviceContext*	GetContext() const { return m_context.Get(); }
		ID3D11RenderTargetView* GetRTV() const { return m_renderTargetView.Get(); }
		ID3D11DepthStencilView* GetDSV() const { return m_depthStencilView.Get(); }

	private:
		bool InitDirect3D(HWND mainWindow, int screenWidth, int screenHeight);
		bool CreateRenderTargetView();
		bool CreateDepthBuffer(int screenWidth, int screenHeight);

	private:
		ComPtr<ID3D11Device>		   m_device;
		ComPtr<ID3D11DeviceContext>	   m_context;
		ComPtr<IDXGISwapChain>		   m_swapChain;
		ComPtr<ID3D11RenderTargetView> m_renderTargetView;

		// depth stencil
		ComPtr<ID3D11Texture2D>		   m_depthTexture;
		ComPtr<ID3D11DepthStencilView> m_depthStencilView;
	};

} // namespace My
