#pragma once
#include <directxtk/SimpleMath.h>
#include <array>
#include <cstddef>
#include "DirectionalLight.h"
#include "PointLight.h"

namespace My
{
	using DirectX::SimpleMath::Matrix;

	struct FrameRenderData
	{
		Matrix									   view;
		Matrix									   projection;
		DirectionalLight						   directionalLight;
		std::array<PointLight, MaxPointLightCount> pointLights{};
		std::size_t								   pointLightCount = 0;
	};
} // namespace My