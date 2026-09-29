#include <algorithm>
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

	void Material::SetEmissiveColor(const Vector3& newEmissiveColor)
	{
		m_emissiveColor = newEmissiveColor;
	}

	void Material::SetRimColor(const Vector3& newRimColor)
	{
		m_rimColor = newRimColor;
	}

	void Material::SetEmissiveIntensity(float newEmissiveIntensity)
	{
		m_emissiveIntensity = newEmissiveIntensity;
	}

	void Material::SetRimIntensity(float newRimIntensity)
	{
		m_rimIntensity = newRimIntensity;
	}

	void Material::SetRimPower(float newRimPower)
	{
		m_rimPower = newRimPower;
	}

	void Material::SetEmissiveMultiplier(float newEmissiveMultiplier)
	{
		m_runtimeEmissiveMultiplier = newEmissiveMultiplier;
	}

	void Material::SetRoughness(float newRoughness)
	{
		// roughness가 0에 가까우면 exponent가 무한대가 될 수도 있음
		m_roughness = std::clamp(newRoughness, 0.05f, 1.0f);
	}

	void Material::SetSpecular(float newSpecular)
	{
		m_specular = std::clamp(newSpecular, 0.0f, 1.0f);
	}

} // namespace My
