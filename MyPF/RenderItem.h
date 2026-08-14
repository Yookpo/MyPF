#pragma once
#include <directxtk/SimpleMath.h>

namespace My
{
	using DirectX::SimpleMath::Matrix;

	class Mesh;

	struct RenderItem
	{
		const Mesh* mesh{ nullptr };
		Matrix world{ Matrix() };
	};
}

