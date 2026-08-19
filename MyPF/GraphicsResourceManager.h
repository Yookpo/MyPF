#pragma once
#include <d3d11.h>
#include <wrl.h>	// comptr
#include <cstdint>
#include <vector>
#include "BufferHandle.h"

namespace My
{
	using Microsoft::WRL::ComPtr;

	class GraphicsDevice;

	class GraphicsResourceManager
	{
	public:
		GraphicsResourceManager() = default;
		GraphicsResourceManager(const GraphicsResourceManager&) = delete;
		GraphicsResourceManager& operator = (const GraphicsResourceManager&) = delete;

		bool Initialize(GraphicsDevice&);
		ID3D11Buffer* GetBuffer(BufferHandle) const;

	private:
		struct BufferResource
		{
			ComPtr<ID3D11Buffer> buffer;
			uint32_t byteWidth = 0;
		};

	private:
		GraphicsDevice* m_graphicsDevice = nullptr;
		std::vector<BufferResource> m_buffers;
	};
}



