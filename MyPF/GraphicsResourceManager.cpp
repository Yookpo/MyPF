#include "GraphicsResourceManager.h"
#include "GraphicsDevice.h"

namespace My
{
	bool GraphicsResourceManager::Initialize(GraphicsDevice& graphicsDevice)
	{
		// 이미 초기화가 됐다면
		if (m_graphicsDevice)
		{
			return false;
		}

		if (!graphicsDevice.GetDevice() || !graphicsDevice.GetContext())
		{
			return false;
		}

		m_graphicsDevice = &graphicsDevice;

		return true;
	}

	ID3D11Buffer* GraphicsResourceManager::GetBuffer(BufferHandle bufferHandle) const
	{
		if (!m_graphicsDevice)
		{
			return nullptr;
		}

		if (!bufferHandle.IsValid())
		{
			return nullptr;
		}

		uint32_t HandleIndex = bufferHandle.GetIndex();

		if (HandleIndex >= m_buffers.size())
		{
			return nullptr;
		}

		const BufferResource& bResource = m_buffers[HandleIndex];

		return bResource.buffer.Get();
	}
}


