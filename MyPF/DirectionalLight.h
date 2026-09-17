#pragma once
#include <directxtk/SimpleMath.h>

namespace My
{
	using DirectX::SimpleMath::Vector3;

	struct DirectionalLight
	{
		// 한밤 골목의 하늘광. 골목 위 좁은 하늘에서 거의 수직으로 떨어져 젖은 바닥을 훑는다.
		// 셰이더가 normalize하지 않으므로 여기서 단위 벡터로 넣는다.
		Vector3 direction = Vector3(0.0f, -0.932f, 0.362f); // 빛의 방향
		float	ambientStrength = 0.045f;				   // 주변광 세기
		Vector3 color = Vector3(0.45f, 0.60f, 1.0f);	   // 빛의 RGB 색상
		float	intensity = 0.18f;						   // 빛의 세기
	};
} // namespace My