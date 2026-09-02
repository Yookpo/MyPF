#pragma once
#include <directxtk/SimpleMath.h>
#include <cstddef>

namespace My
{
	inline constexpr std::size_t MaxPointLightCount = 8;

	using DirectX::SimpleMath::Vector3;

	struct PointLight
	{
		Vector3 position = Vector3(0.0f, 0.0f, 0.0f); // 점광의 위치
		float	range = 2.0f;						  // 점광의 범위
		Vector3 color = Vector3(1.0f);				  // 점광의 RGB 색상
		float	intensity = 1.0f;					  // 점광의 세기
		bool	isEnabled = false;					  // 점등 여부
	};
} // namespace My