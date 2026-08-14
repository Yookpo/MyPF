#pragma once
#include <directxtk/SimpleMath.h>

namespace My
{
	using DirectX::SimpleMath::Matrix;

	struct FrameRenderData
	{
		Matrix view;
		Matrix projection;
	};
}