#pragma once
#include <directxtk/SimpleMath.h>
#include "DirectionalLight.h"

namespace My
{
	using DirectX::SimpleMath::Matrix;

	struct FrameRenderData
	{
		Matrix			 view;
		Matrix			 projection;
		DirectionalLight directionalLight;
	};
} // namespace My