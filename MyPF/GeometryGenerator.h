#pragma once
#include <directxtk/SimpleMath.h>
#include <vector>

namespace My
{
	using DirectX::SimpleMath::Vector2;
	using DirectX::SimpleMath::Vector3;

	struct Vertex {
		Vector3 position;
		Vector3 normal;	// 지금은 컬러
	};

	struct MeshData {
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
	};


	class GeometryGenerator
	{
	public:
		static MeshData MakeTriangle();


	};
}
