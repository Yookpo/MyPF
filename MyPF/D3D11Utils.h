#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxtk/SimpleMath.h>
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
			int screenHeight, ComPtr<ID3D11DepthStencilView>& depthStencilView, ComPtr<ID3D11DepthStencilState>& depthStencilState);

		static bool CreateVertexShaderAndInputLayout(
			ComPtr<ID3D11Device>& device, const wstring& fileName,
			const vector<D3D11_INPUT_ELEMENT_DESC>& inputElements,
			ComPtr<ID3D11VertexShader>& m_vertexShader,
			ComPtr<ID3D11InputLayout>& m_inputLayout
		);

		static bool CreatePixelShader(ComPtr<ID3D11Device>& device,
			const wstring& fileName, ComPtr<ID3D11PixelShader>& m_pixelShader);

		static bool CreateIndexBuffer(ID3D11Device* device,
			const vector<uint32_t>& indices, ComPtr<ID3D11Buffer>& indexBuffer);

		template <typename T_VERTEX>
		static bool CreateVertexBuffer(ID3D11Device* device,
			const vector<T_VERTEX>& vertices,
			ComPtr<ID3D11Buffer>& vertexBuffer)
		{
			if (!device || !vertices.size())
			{
				return false;
			}

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
				OutputDebugStringW(L"CreateVertexBuffer() failed");
				return false;
			}

			return true;
		}

		template <typename T_CONSTANT>
		static bool CreateConstantBuffer(ComPtr<ID3D11Device>& device,
			const T_CONSTANT& constantBufferData,
			ComPtr<ID3D11Buffer>& constantBuffer)
		{
			if (!device)
			{
				return false;
			}

			static_assert((sizeof(T_CONSTANT) % 16) == 0,
				"Constant Buffer size must be 16-byte aligned");

			D3D11_BUFFER_DESC cbDesc;
			cbDesc.ByteWidth = sizeof(constantBufferData);
			cbDesc.Usage = D3D11_USAGE_DYNAMIC;
			cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			cbDesc.MiscFlags = 0;
			cbDesc.StructureByteStride = 0;

			D3D11_SUBRESOURCE_DATA initData;
			initData.pSysMem = &constantBufferData;
			initData.SysMemPitch = 0;
			initData.SysMemSlicePitch = 0;

			auto hr = device->CreateBuffer(&cbDesc, &initData,
				constantBuffer.GetAddressOf());
			if (FAILED(hr)) {
				OutputDebugStringW(L"CreateConstantBuffer() CreateBuffer failed()");
				return false;
			}

			return true;
		}
		template <typename T_DATA>
		static bool UpdateBuffer(ComPtr<ID3D11DeviceContext>& context,
			const T_DATA& bufferData,
			ComPtr<ID3D11Buffer>& buffer) {

			if (!buffer || !context) {
				OutputDebugStringW(L"UpdateBuffer() buffer was not initialized.");
				return false;
			}

			D3D11_MAPPED_SUBRESOURCE ms = {};
			auto hr = context->Map(buffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
			if (FAILED(hr))
			{
				OutputDebugStringW(L"Map() failed.");
				return false;
			}
			memcpy(ms.pData, &bufferData, sizeof(bufferData));
			context->Unmap(buffer.Get(), NULL);

			return true;
		}
	};
}



