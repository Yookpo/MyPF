#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <array>
#include <windows.h>
#include <wrl.h>	// comptr

namespace My
{
	using Microsoft::WRL::ComPtr;
	using std::shared_ptr;
	using std::vector;
	using std::wstring;

	class D3D11Utils
	{
	public:
		static bool CreateDepthBuffer(
			ComPtr<ID3D11Device>& device, int screenWidth,
			int screenHeight, ComPtr<ID3D11DepthStencilView>& depthStencilView);

		static void CreateVertexShaderAndInputLayout(
			ComPtr<ID3D11Device>& device, const wstring& fileName,
			const vector<D3D11_INPUT_ELEMENT_DESC>& inputElements,
			ComPtr<ID3D11VertexShader>& m_vertexShader,
			ComPtr<ID3D11InputLayout>& m_inputLayout
		);

		static void CreatePixelShader(ComPtr<ID3D11Device>& device,
			const wstring& fileName, ComPtr<ID3D11PixelShader>& m_pixelShader);

		static void CreateIndexBuffer(ComPtr<ID3D11Device>& device,
			const vector<uint32_t>& indices, ComPtr<ID3D11Buffer>& indexBuffer);

		template <typename T_VERTEX>
		static void CreateVertexBuffer(ComPtr<ID3D11Device>& device,
			const vector<T_VERTEX>& vertices,
			ComPtr<ID3D11Buffer>& vertexBuffer)
		{
			D3D11_BUFFER_DESC bufferDesc;
			ZeroMemory(&bufferDesc, sizeof(bufferDesc));
			bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
			bufferDesc.ByteWidth = UINT(sizeof(T_VERTEX) * vertices.size());
			bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bufferDesc.CPUAccessFlags = 0;
			bufferDesc.StructureByteStride = sizeof(T_VERTEX);

			D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
			vertexBufferData.pSysMem = vertices.data();
			vertexBufferData.SysMemPitch = 0;
			vertexBufferData.SysMemSlicePitch = 0;

			const HRESULT hr = device->CreateBuffer(&bufferDesc, &vertexBufferData,
				vertexBuffer.GetAddressOf());

			if (FAILED(hr))
			{
				OutputDebugStringW(L"CreateBuffer() failed");
			}
		}



	};
}



