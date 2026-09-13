#pragma once
#include <string>
#include <directxtk/SimpleMath.h>

namespace My
{
	using DirectX::SimpleMath::Vector3;

	class Scene;
	class AssetManager;
	class PointLightSequence;
	class Mesh;
	class Texture;

	// 네온 사인 하나를 구성하는 데 필요한 입력 데이터
	// color 필드 하나 공유
	struct NeonSignDesc
	{
		std::string name;		   // GameObject/Material 이름에 쓸 base Key
		Vector3		glowPosition;  // 발광 메쉬의 월드 위치
		Vector3		glowSCale;	   // 발광 메쉬 크기
		Vector3		lightPosition; // Point Light 위치 (간판과 같은 자리 아닐 수도)
		float		lightRange;
		float		lightIntensity;
		Vector3		color; // Emissive Color , Point Light Color 공용
		float		emissiveIntensity;
	};

	class NeonSignFactory
	{
	public:
		// Desc대로 발광 메쉬 GameObject + Point Light GameObject + Material 만들고
		// sequence에 등록 한다. 실패 시 false
		static bool Create(Scene& scene, AssetManager& assetManager, PointLightSequence& sequence, const Mesh* glowMesh,
			const Texture* baseTexture, const NeonSignDesc& desc);
	};

} // namespace My
