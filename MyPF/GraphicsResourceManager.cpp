#include "GraphicsResourceManager.h"
#include "GraphicsDevice.h"
#include <utility>
#include <cstring>

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

	BufferHandle GraphicsResourceManager::CreateConstantBufferInternal(const void* data, uint32_t byteWidth)
	{
		if (!m_graphicsDevice || !m_graphicsDevice->GetDevice() || !data || (byteWidth == 0 || (byteWidth % 16 != 0)))
		{
			return BufferHandle{};
		}

		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = byteWidth;
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = data;
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		BufferResource newResource;

		auto hr = m_graphicsDevice->GetDevice()->CreateBuffer(&cbDesc, &initData, newResource.buffer.GetAddressOf());
		newResource.byteWidth = byteWidth;

		if (FAILED(hr))
		{
			OutputDebugStringW(L"CreateConstantBuffer() failed()");
			return BufferHandle{};
		}

		uint32_t newIndex = static_cast<uint32_t>(m_buffers.size());
		m_buffers.push_back(std::move(newResource));

		return BufferHandle(newIndex);
	}

	bool GraphicsResourceManager::UpdateBufferInternal(const BufferHandle& bufferHandle, const void* data, uint32_t byteWidth)
	{
		if (!m_graphicsDevice)
		{
			return false;
		}

		if (!m_graphicsDevice->GetContext() || !data || byteWidth == 0 || !bufferHandle.IsValid() || bufferHandle.GetIndex() >= m_buffers.size())
		{
			return false;
		}

		uint32_t Index = bufferHandle.GetIndex();
		BufferResource& updateResource = m_buffers[Index];
		ID3D11DeviceContext* Context = m_graphicsDevice->GetContext();

		if (!updateResource.buffer.Get() || (updateResource.byteWidth != byteWidth))
		{
			return false;
		}

		D3D11_MAPPED_SUBRESOURCE ms = {};
		auto hr = Context->Map(updateResource.buffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);

		if (FAILED(hr))
		{
			OutputDebugStringW(L"Map() failed.");
			return false;
		}

		std::memcpy(ms.pData, data, byteWidth);
		Context->Unmap(updateResource.buffer.Get(), NULL);

		return true;
	}
}


