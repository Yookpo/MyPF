#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxtk/SimpleMath.h>
#include <iostream>
#include <memory>
#include <limits>
#include <string>
#include <vector>
#include <array>
#include <windows.h>
#include <wrl.h> // comptr

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
			ID3D11Device* device, int screenWidth,
			int screenHeight, ComPtr<ID3D11DepthStencilView>& depthStencilView, ComPtr<ID3D11DepthStencilState>& depthStencilState);

		static bool CreateDepthStencilState(ID3D11Device* device, ComPtr<ID3D11DepthStencilState>& depthStencilState);

		static bool CreateVertexShaderAndInputLayout(
			ID3D11Device* device, const wstring& fileName,
			const vector<D3D11_INPUT_ELEMENT_DESC>& inputElements,
			ComPtr<ID3D11VertexShader>&				m_vertexShader,
			ComPtr<ID3D11InputLayout>&				m_inputLayout);

		static bool CreatePixelShader(ID3D11Device* device,
			const wstring& fileName, ComPtr<ID3D11PixelShader>& m_pixelShader);

		static bool CreateImmutableBuffer(
			ID3D11Device* device, const void* data, uint32_t byteWidth,
			UINT bindFlags, ComPtr<ID3D11Buffer>& buffer);

		static bool CreateIndexBuffer(ID3D11Device* device,
			const vector<uint32_t>& indices, ComPtr<ID3D11Buffer>& indexBuffer);

		template <typename T_VERTEX>
		static bool CreateVertexBuffer(ID3D11Device* device,
			const vector<T_VERTEX>&					 vertices,
			ComPtr<ID3D11Buffer>&					 vertexBuffer)
		{
			const uint32_t maxValue = (std::numeric_limits<uint32_t>::max)();

			if (!device || !vertices.size() || vertices.size() > maxValue / sizeof(T_VERTEX))
			{
				return false;
			}

			const uint32_t byteWidth = static_cast<uint32_t>(sizeof(T_VERTEX) * vertices.size());

			return CreateImmutableBuffer(device, vertices.data(), byteWidth, D3D11_BIND_VERTEX_BUFFER, vertexBuffer);
		}

		static bool CreateConstantBuffer(
			ID3D11Device*		  device,
			const void*			  data,
			uint32_t			  byteWidth,
			ComPtr<ID3D11Buffer>& constantBuffer);

		template <typename T_CONSTANT>
		static bool CreateConstantBuffer(ID3D11Device* device,
			const T_CONSTANT&						   constantBufferData,
			ComPtr<ID3D11Buffer>&					   constantBuffer)
		{
			static_assert((sizeof(T_CONSTANT) % 16) == 0,
				"Constant Buffer size must be 16-byte aligned");

			return CreateConstantBuffer(
				device,
				&constantBufferData,
				static_cast<uint32_t>(sizeof(T_CONSTANT)),
				constantBuffer);
		}

		static bool UpdateBuffer(
			ID3D11DeviceContext* context,
			const void*			 data,
			uint32_t			 byteWidth,
			ID3D11Buffer*		 buffer);

		template <typename T_DATA>
		static bool UpdateBuffer(ID3D11DeviceContext* context,
			const T_DATA&							  bufferData,
			ID3D11Buffer*							  buffer)
		{

			return UpdateBuffer(context, &bufferData, static_cast<uint32_t>(sizeof(T_DATA)), buffer);
		}

		static bool CreateTexture(ID3D11Device* device, const std::string& filename,
			ComPtr<ID3D11Texture2D>&		  texture,
			ComPtr<ID3D11ShaderResourceView>& textureResourceView);
	};
} // namespace My
