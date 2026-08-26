#pragma once
#include <directxtk/SimpleMath.h>

namespace My
{
	using DirectX::SimpleMath::Vector3;

	class Texture;

	class Material
	{
	public:
		Material()
			: m_albedoTexture(nullptr), m_baseColor{ 1.0f } {}
		Material(const Material&) = delete;
		Material& operator=(const Material&) = delete;

		void		   SetAlbedoTexture(const Texture*);
		const Texture* GetAlbedoTexture() const { return m_albedoTexture; }

		void		   SetBaseColor(const Vector3&);
		const Vector3& GetBaseColor() const { return m_baseColor; }

	private:
		const Texture* m_albedoTexture;
		Vector3		   m_baseColor;
	};

} // namespace My
