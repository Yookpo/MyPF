#pragma once
#include <directxtk/SimpleMath.h>

namespace My
{
	using DirectX::SimpleMath::Vector3;

	struct DirectionalLight
	{
		Vector3 direction = Vector3(0.0f, -0.5f, 1.0f);	// 빛의 방향
		Vector3 color = Vector3(1.0f);	// 빛의 RGB 색상
		float intensity = 1.0f;	// 빛의 세기
	};
}