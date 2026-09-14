#include "NeonSign.h"
#include "Scene.h"
#include "AssetManager.h"
#include "PointLightSequence.h"
#include "GameObject.h"
#include "Material.h"

namespace My
{
	bool NeonSignFactory::Create(Scene& scene, AssetManager& assetManager, PointLightSequence& sequence,
		const Mesh* glowMesh, const Texture* baseTexture, const NeonSignDesc& desc)
	{
		// Neon Object 생성

		// 1. Material 생성
		auto neonMat = assetManager.CreateMaterial(desc.name + "_Mat");

		if (!neonMat)
		{
			return false;
		}

		neonMat->SetAlbedoTexture(baseTexture);
		neonMat->SetBaseColor(desc.color * 0.12f);
		neonMat->SetEmissiveColor(desc.color);
		neonMat->SetEmissiveIntensity(desc.emissiveIntensity);

		// 2. Point Light GameObject
		GameObject& lightObject = scene.CreatePointLightObject(desc.name + "_Light");
		lightObject.GetTransform().SetPosition(desc.lightPosition);
		lightObject.GetPointLightComponent().SetColor(desc.color);
		lightObject.GetPointLightComponent().SetRange(desc.lightRange);
		lightObject.GetPointLightComponent().SetIntensity(desc.lightIntensity);
		lightObject.GetPointLightComponent().SetEnabled(false);

		// 3. 시퀀스 등록
		if (!sequence.AddSequenceEntry(lightObject, neonMat))
		{
			return false;
		}

		// 4. Neon GameObject
		GameObject& glowObject = scene.CreateGameObject(desc.name + "_Glow");
		glowObject.GetTransform().SetPosition(desc.glowPosition);
		glowObject.GetTransform().SetScale(desc.glowScale);
		glowObject.GetMeshComponent().SetMesh(glowMesh);
		glowObject.GetMeshComponent().SetMaterial(neonMat);

		return true;
	}

} // namespace My
