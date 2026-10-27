#pragma once
#include <directxtk/SimpleMath.h>
#include <array>
#include <cstddef>
#include "PostProcessSettings.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "TextureHandle.h"

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
		TextureHandle							   environmentMap; // 하늘 큐브맵. 무효면 스카이박스 안그림
	};
} // namespace My