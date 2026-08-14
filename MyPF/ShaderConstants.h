#pragma once
#include <directxtk/SimpleMath.h>

namespace My
{
	using DirectX::SimpleMath::Matrix;

	struct BasicVertexConstantData
	{
		Matrix model;
		Matrix view;
		Matrix projection;
	};
}

