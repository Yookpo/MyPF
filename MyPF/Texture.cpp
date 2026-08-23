#include "Texture.h"
#include "GraphicsResourceManager.h"

namespace My
{
	bool Texture::Initialize(GraphicsResourceManager& resourceManager, const std::string& fileName)
	{
		TextureHandle textureHandle = resourceManager.CreateTexture(fileName);

		if (!textureHandle.IsValid())
		{
			return false;
		}

		m_textureHandle = textureHandle;

		return true;
	}
}


