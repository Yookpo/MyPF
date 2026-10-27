#pragma once
#include <directxtk/SimpleMath.h>

namespace My
{
	using DirectX::SimpleMath::Vector3;

	// 한밤 골목의 하늘광. 거의 수직으로 떨어져 젖은 바닥을 훑고, 옆벽은 N·L ≈ 0이라 받지 않는다(룩이 이 성질에 의존).
	// direction은 CPU·셰이더가 모두 정규화하지만 의미가 분명하도록 단위 벡터로 둔다.
	struct DirectionalLight
	{
		Vector3 direction = Vector3(0.0f, -0.932f, 0.362f); // 빛이 나아가는 방향
		float	ambientStrength = 0.045f;					// 주변광(하늘 radiance). π 규약에서도 × π 하지 않는다
		float	environmentIntensity = 0.25f;				// 환경맵(하늘·반사) 밝기 배율
		Vector3 color = Vector3(0.45f, 0.60f, 1.0f);		// 빛의 RGB 색상(sRGB)
		float	intensity = 0.18f * DirectX::XM_PI;			// 빛의 세기. π 규약(diffuse = albedo / π) 전환 전 값 × π
	};
} // namespace My