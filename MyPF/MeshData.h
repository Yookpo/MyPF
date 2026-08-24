#pragma once
#include <directxtk/SimpleMath.h>
#include <vector>
#include <cstdint>

// 순수 Geometry 표현
namespace My
{
	using DirectX::SimpleMath::Vector3;
	using DirectX::SimpleMath::Vector2;

	struct Vertex
	{
		Vector3 position;
		Vector3 color;
		Vector3 normal;
		Vector2 uv{ 0.0f,0.0f };
	};


	struct MeshData
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
	};

}

