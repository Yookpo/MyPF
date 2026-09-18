#include "Texture.h"
#include "GraphicsResourceManager.h"

namespace My
{
	static DXGI_FORMAT ToDxgiFormat(TextureType texType)
	{
		switch (texType)
		{
			case TextureType::Albedo:
				return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			case TextureType::NormalMap:
				return DXGI_FORMAT_R8G8B8A8_UNORM;
			case TextureType::Data:
				return DXGI_FORMAT_R8G8B8A8_UNORM;
		}

		return DXGI_FORMAT_UNKNOWN;
	}

	bool Texture::Initialize(GraphicsResourceManager& resourceManager, const std::string& fileName, TextureType texType)
	{
		TextureHandle textureHandle = resourceManager.CreateTexture(fileName, ToDxgiFormat(texType));

		if (!textureHandle.IsValid())
		{
			return false;
		}

		m_textureHandle = textureHandle;

		return true;
	}
} // namespace My
