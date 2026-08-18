#pragma once
#include <directxtk/SimpleMath.h>
#include <vector>
#include <cstdint>

namespace My
{
	using DirectX::SimpleMath::Vector3;


	struct Vertex 
	{
		Vector3 position;
		Vector3 color;
		Vector3 normal;
	};

	struct MeshData 
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
	};

}

