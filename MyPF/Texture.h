#pragma once
#include "TextureHandle.h"
#include "TextureType.h"
#include <string>

namespace My
{
	class GraphicsResourceManager;

	class Texture
	{
	public:
		Texture() = default;
		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;

		bool Initialize(GraphicsResourceManager& resourceManager, const std::string& fileName, TextureType texType);

		TextureHandle GetTextureHandle() const { return m_textureHandle; }

	private:
		TextureHandle	   m_textureHandle;
	};
} // namespace My
