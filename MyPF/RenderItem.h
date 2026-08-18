#pragma once
#include <directxtk/SimpleMath.h>

namespace My
{
	using DirectX::SimpleMath::Matrix;

	class Mesh;
	class Material;

	struct RenderItem
	{
		const Mesh* mesh{ nullptr };
		const Material* material{ nullptr };
		Matrix world{ Matrix() };
	};
}

