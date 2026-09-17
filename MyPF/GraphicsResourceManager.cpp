#include "GraphicsResourceManager.h"
#include "GraphicsDevice.h"
#include "D3D11Utils.h"
#include <utility>

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

	ID3D11ShaderResourceView* GraphicsResourceManager::GetSRV(TextureHandle textureHandle) const
	{
		if (!m_graphicsDevice)
		{
			return nullptr;
		}

		if (!textureHandle.IsValid())
		{
			return nullptr;
		}

		uint32_t HandleIndex = textureHandle.GetIndex();

		if (HandleIndex >= m_textures.size())
		{
			return nullptr;
		}

		const TextureResource& tResource = m_textures[HandleIndex];

		return tResource.textureSRV.Get();
	}

	ID3D11RenderTargetView* GraphicsResourceManager::GetRTV(TextureHandle textureHandle) const
	{
		if (!m_graphicsDevice)
		{
			return nullptr;
		}

		if (!textureHandle.IsValid())
		{
			return nullptr;
		}

		uint32_t HandleIndex = textureHandle.GetIndex();

		if (HandleIndex >= m_textures.size())
		{
			return nullptr;
		}

		const TextureResource& tResource = m_textures[HandleIndex];

		return tResource.textureRTV.Get();
	}

	TextureHandle GraphicsResourceManager::CreateTexture(const std::string& filename)
	{
		if (!m_graphicsDevice || !m_graphicsDevice->GetDevice() || filename.empty())
		{
			return TextureHandle{};
		}

		TextureResource tResource{};

		if (!D3D11Utils::CreateTexture(
				m_graphicsDevice->GetDevice(), filename, tResource.texture, tResource.textureSRV))
		{
			return TextureHandle{};
		}

		uint32_t newIndex = static_cast<uint32_t>(m_textures.size());
		m_textures.push_back(std::move(tResource));

		return TextureHandle(newIndex);
	}

	TextureHandle GraphicsResourceManager::CreateRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format)
	{
		if (!m_graphicsDevice || !m_graphicsDevice->GetDevice())
		{
			return TextureHandle{};
		}

		TextureResource tResource{};
		tResource.textureFormat = format;

		if (!D3D11Utils::CreateRenderTargetTexture(m_graphicsDevice->GetDevice(), width, height,
				tResource.textureFormat, tResource.texture, tResource.textureRTV, tResource.textureSRV))
		{
			return TextureHandle{};
		}

		uint32_t newIndex = static_cast<uint32_t>(m_textures.size());
		m_textures.push_back(std::move(tResource));

		return TextureHandle(newIndex);
	}

	bool GraphicsResourceManager::ResizeRenderTarget(TextureHandle textureHandle, uint32_t width, uint32_t height)
	{
		if (!m_graphicsDevice || !m_graphicsDevice->GetDevice())
		{
			return false;
		}

		if (!textureHandle.IsValid() || textureHandle.GetIndex() >= m_textures.size())
		{
			return false;
		}

		TextureResource& slot = m_textures[textureHandle.GetIndex()];

		// 파일 텍스처는 안본다
		if (!slot.textureRTV.Get())
		{
			return false;
		}

		ComPtr<ID3D11Texture2D>			 tex;
		ComPtr<ID3D11ShaderResourceView> texSRV;
		ComPtr<ID3D11RenderTargetView>	 texRTV;

		// 리사이즈된 객체를 임시객체로 변환
		if (!D3D11Utils::CreateRenderTargetTexture(
				m_graphicsDevice->GetDevice(), width, height, slot.textureFormat, tex, texRTV, texSRV))
		{
			return false;
		}

		slot.texture = std::move(tex);
		slot.textureSRV = std::move(texSRV);
		slot.textureRTV = std::move(texRTV);

		return true;
	}

	BufferHandle GraphicsResourceManager::CreateIndexBuffer(const std::vector<uint32_t>& indices)
	{
		auto maxValue = (std::numeric_limits<uint32_t>::max)();

		if (indices.empty() || (indices.size() > (maxValue / sizeof(uint32_t))))
		{
			return BufferHandle{};
		}

		auto byte = sizeof(uint32_t) * indices.size();

		return CreateImmutableBufferInternal(indices.data(), static_cast<uint32_t>(byte), D3D11_BIND_INDEX_BUFFER);
	}

	BufferHandle GraphicsResourceManager::CreateImmutableBufferInternal(
		const void* data, uint32_t byteWidth, UINT bindFlags)
	{
		if (!m_graphicsDevice || !m_graphicsDevice->GetDevice() || !data || (byteWidth == 0) || (bindFlags == 0))
		{
			return BufferHandle{};
		}

		BufferResource newResource;

		if (!D3D11Utils::CreateImmutableBuffer(
				m_graphicsDevice->GetDevice(), data, byteWidth, bindFlags, newResource.buffer))
		{
			return BufferHandle{};
		}

		newResource.byteWidth = byteWidth;
		newResource.cpuWritable = false;

		uint32_t newIndex = static_cast<uint32_t>(m_buffers.size());
		m_buffers.push_back(std::move(newResource));

		return BufferHandle(newIndex);
	}

	BufferHandle GraphicsResourceManager::CreateConstantBufferInternal(const void* data, uint32_t byteWidth)
	{
		if (!m_graphicsDevice || !m_graphicsDevice->GetDevice() || !data || (byteWidth == 0 || (byteWidth % 16 != 0)))
		{
			return BufferHandle{};
		}

		BufferResource newResource;

		if (!D3D11Utils::CreateConstantBuffer(m_graphicsDevice->GetDevice(), data, byteWidth, newResource.buffer))
		{
			return BufferHandle{};
		}

		newResource.byteWidth = byteWidth;
		newResource.cpuWritable = true;

		const uint32_t newIndex = static_cast<uint32_t>(m_buffers.size());
		m_buffers.push_back(std::move(newResource));

		return BufferHandle(newIndex);
	}

	bool GraphicsResourceManager::UpdateBufferInternal(
		const BufferHandle& bufferHandle, const void* data, uint32_t byteWidth)
	{
		if (!m_graphicsDevice || !m_graphicsDevice->GetContext() || !data || byteWidth == 0 || !bufferHandle.IsValid()
			|| bufferHandle.GetIndex() >= m_buffers.size())
		{
			return false;
		}

		uint32_t		Index = bufferHandle.GetIndex();
		BufferResource& updateResource = m_buffers[Index];

		if (!updateResource.buffer || (updateResource.byteWidth != byteWidth) || !updateResource.cpuWritable)
		{
			return false;
		}

		return D3D11Utils::UpdateBuffer(m_graphicsDevice->GetContext(), data, byteWidth, updateResource.buffer.Get());
	}
} // namespace My
