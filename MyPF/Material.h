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
			: m_albedoTexture(nullptr), m_baseColor{ 1.0f }, m_emissiveColor{ 1.0f }, m_emissiveIntensity{ 0.0f }, m_runtimeEmissiveMultiplier{ 1.0f } {}
		Material(const Material&) = delete;
		Material& operator=(const Material&) = delete;

		void		   SetAlbedoTexture(const Texture*);
		const Texture* GetAlbedoTexture() const { return m_albedoTexture; }

		void		   SetBaseColor(const Vector3&);
		const Vector3& GetBaseColor() const { return m_baseColor; }

		void		   SetEmissiveColor(const Vector3&);
		const Vector3& GetEmissiveColor() const { return m_emissiveColor; }

		void  SetEmissiveIntensity(float);
		float GetEmissiveIntensity() const { return m_emissiveIntensity; }

		void  SetEmissiveMultiplier(float);
		float GetEmissiveMultiplier() const { return m_runtimeEmissiveMultiplier; }
		float GetEffectiveEmissiveIntensity() const { return m_emissiveIntensity * m_runtimeEmissiveMultiplier; }

	private:
		const Texture* m_albedoTexture;
		Vector3		   m_baseColor;
		Vector3		   m_emissiveColor;
		float		   m_emissiveIntensity;			// Editor가 설정하는 원본 밝기
		float		   m_runtimeEmissiveMultiplier; // Sequence가 조절하는 현재 출력 배율	(0.0 꺼짐 / 1.0 정상출력)
	};

} // namespace My
