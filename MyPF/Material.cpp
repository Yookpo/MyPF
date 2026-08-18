#include "Material.h"
#include "Texture.h"

namespace My
{
	void Material::SetAlbedoTexture(const Texture* newTexture)
	{
		m_albedoTexture = newTexture;
	}

	void Material::SetBaseColor(const Vector3& newColor)
	{
		m_baseColor = newColor;
	}

}

