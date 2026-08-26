#pragma once
#include "TextureHandle.h"
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

		bool Initialize(GraphicsResourceManager&, const std::string&);

		TextureHandle GetTextureHandle() const { return m_textureHandle; }

	private:
		TextureHandle m_textureHandle;
	};
} // namespace My
