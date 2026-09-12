#pragma once
#include <d3d11.h>
#include <wrl.h> // comptr
#include <cstdint>
#include <vector>
#include <string>
#include <limits>
#include "BufferHandle.h"
#include "TextureHandle.h"

namespace My
{
	using Microsoft::WRL::ComPtr;

	class GraphicsDevice;

	class GraphicsResourceManager
	{
	public:
		GraphicsResourceManager() = default;
		GraphicsResourceManager(const GraphicsResourceManager&) = delete;
		GraphicsResourceManager& operator=(const GraphicsResourceManager&) = delete;

		bool					  Initialize(GraphicsDevice&);
		ID3D11Buffer*			  GetBuffer(BufferHandle) const;
		ID3D11ShaderResourceView* GetSRV(TextureHandle) const;

		TextureHandle CreateTexture(const std::string& filename);

		template <typename T_VERTEX>
		BufferHandle CreateVertexBuffer(const std::vector<T_VERTEX>& vertices)
		{
			const uint32_t maxValue = (std::numeric_limits<uint32_t>::max)();

			if (vertices.empty() || (vertices.size() > (maxValue / sizeof(T_VERTEX))))
			{
				return BufferHandle{};
			}

			auto byte = sizeof(T_VERTEX) * vertices.size();

			return CreateImmutableBufferInternal(vertices.data(), static_cast<uint32_t>(byte), D3D11_BIND_VERTEX_BUFFER);
		}

		BufferHandle CreateIndexBuffer(const std::vector<uint32_t>& indices);

		template <typename T_CONSTANT>
		BufferHandle CreateConstantBuffer(const T_CONSTANT& initialData)
		{
			static_assert((sizeof(T_CONSTANT) % 16) == 0,
				"Constant Buffer size must be 16-byte aligned");

			return CreateConstantBufferInternal(&initialData, static_cast<uint32_t>(sizeof(T_CONSTANT)));
		}

		template <typename T_DATA>
		bool UpdateBuffer(const BufferHandle& bufferHandle, const T_DATA& bufferData)
		{
			static_assert((sizeof(T_DATA) % 16) == 0,
				"Constant Buffer size must be 16-byte aligned");

			return UpdateBufferInternal(bufferHandle, &bufferData, static_cast<uint32_t>(sizeof(T_DATA)));
		}

	private:
		struct BufferResource
		{
			ComPtr<ID3D11Buffer> buffer;
			uint32_t			 byteWidth = 0;		  // 버퍼 생성 시 크기, 갱신할 데이터 크기가 기존 버퍼 크기와 같은 지 검사해야함
			bool				 cpuWritable = false; // 이 버퍼는 CPU에서 갱신이 가능?
		};

		struct TextureResource
		{
			ComPtr<ID3D11Texture2D>			 texture;
			ComPtr<ID3D11ShaderResourceView> textureSRV;
		};

		BufferHandle CreateImmutableBufferInternal(const void* data, uint32_t byteWidth, UINT flag);
		BufferHandle CreateConstantBufferInternal(const void* data, uint32_t byteWidth);
		bool		 UpdateBufferInternal(const BufferHandle& bufferHandle, const void* data, uint32_t byteWidth);

		GraphicsDevice*				 m_graphicsDevice = nullptr;
		std::vector<BufferResource>	 m_buffers;
		std::vector<TextureResource> m_textures;
	};
} // namespace My
