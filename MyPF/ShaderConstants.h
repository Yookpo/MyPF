#pragma once
#include <directxtk/SimpleMath.h>

namespace My
{
	using DirectX::SimpleMath::Matrix;

	struct ObjectConstantData
	{
		Matrix model;
		Matrix invTranspose;
	};

	struct CameraConstantData
	{
		Matrix view;
		Matrix projection;
	};
}

