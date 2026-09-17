#pragma once
#include <directxtk/SimpleMath.h>
#include <array>
#include <cstddef>
#include "PostProcessSettings.h"
#include "DirectionalLight.h"
#include "PointLight.h"

namespace My
{
	using DirectX::SimpleMath::Matrix;
	using DirectX::SimpleMath::Vector3;

	struct FrameRenderData
	{
		Matrix									   view;
		Matrix									   projection;
		DirectionalLight						   directionalLight;
		std::array<PointLight, MaxPointLightCount> pointLights{};
		std::size_t								   pointLightCount = 0;
		Vector3									   cameraPosition;
		PostProcessSettings						   postProcess;
	};
} // namespace My