#include "Texture.h"
#include "D3D11Utils.h"

namespace My
{
	bool Texture::Initialize(ID3D11Device* device, const std::string& fileName)
	{
		if (!D3D11Utils::CreateTexture(device, fileName, m_texture, m_textureResourceView))
		{
			OutputDebugStringW(L"Texture Load Failed");
			return false;
		}

		return true;
	}
}


