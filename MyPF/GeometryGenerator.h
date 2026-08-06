#pragma once
#include <directxtk/SimpleMath.h>
#include <vector>

namespace My
{
	using DirectX::SimpleMath::Vector2;
	using DirectX::SimpleMath::Vector3;

	using DirectX::SimpleMath::Matrix;

	struct BasicVertexConstantData
	{
		Matrix model;
	};

	static_assert((sizeof(BasicVertexConstantData) % 16) == 0,
		"Constant Buffer size must be 16-byte aligned");

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

		// 필요한 지형데이터 입력함수 작성
	};
}
