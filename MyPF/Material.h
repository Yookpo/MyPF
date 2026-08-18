#pragma once

namespace My
{
	class Texture;

	class Material
	{
	public:
		Material() :m_albedoTexture(nullptr) {}
		Material(const Material&) = delete;
		Material& operator = (const Material&) = delete;

		void SetAlbedoTexture(const Texture*);
		const Texture* GetAlbedoTexture() const { return m_albedoTexture; }

	private:
		const Texture* m_albedoTexture;
	};

}

