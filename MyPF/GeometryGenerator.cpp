#include "GeometryGenerator.h"

namespace My
{
	using namespace std;
	using namespace DirectX;
	using namespace DirectX::SimpleMath;


	MeshData GeometryGenerator::MakeTriangle()
	{
		vector<Vector3> positions;
		vector<Vector3> colors;
		vector<Vector3> normals;
		vector<Vector2> coords;

		positions.push_back(Vector3(-1.0f, 0.0f, 0.0f));
		positions.push_back(Vector3(0.0f, 1.0f, 0.0f));
		positions.push_back(Vector3(1.0f, 0.0f, 0.0f));

		colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
		colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
		colors.push_back(Vector3(1.0f, 0.0f, 0.0f));

		normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
		normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
		normals.push_back(Vector3(0.0f, 0.0f, -1.0f));

		coords.push_back(Vector2(0.0f, 1.0f));
		coords.push_back(Vector2(0.5f, 0.0f));
		coords.push_back(Vector2(1.0f, 1.0f));


		MeshData meshData;

		for (int i = 0; i < 3; i++)
		{
			Vertex v;
			v.position = positions[i];
			v.color = colors[i];
			v.normal = normals[i];
			v.uv = coords[i];

			meshData.vertices.push_back(v);
		}
		meshData.indices = {
			0,1,2
		};

		return meshData;
	}

	MeshData GeometryGenerator::MakeCube()
	{
		vector<Vector3> positions;
		vector<Vector3> colors;
		vector<Vector3> normals;
		vector<Vector2> coords;

		const float scale = 1.0f;

		// 윗면
		positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
		positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
		positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
		positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
		colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
		colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
		colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
		colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
		normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
		normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
		normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
		normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
		coords.push_back(Vector2(0.0f, 1.0f));
		coords.push_back(Vector2(0.0f, 0.0f));
		coords.push_back(Vector2(1.0f, 0.0f));
		coords.push_back(Vector2(1.0f, 1.0f));

		// 아랫면
		positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
		positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
		positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
		positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
		colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
		colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
		colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
		colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
		normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
		normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
		normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
		normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
		coords.push_back(Vector2(0.0f, 1.0f));
		coords.push_back(Vector2(0.0f, 0.0f));
		coords.push_back(Vector2(1.0f, 0.0f));
		coords.push_back(Vector2(1.0f, 1.0f));

		// 앞면
		positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
		positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
		positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
		positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
		colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
		colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
		colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
		colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
		normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
		normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
		normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
		normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
		coords.push_back(Vector2(0.0f, 1.0f));
		coords.push_back(Vector2(0.0f, 0.0f));
		coords.push_back(Vector2(1.0f, 0.0f));
		coords.push_back(Vector2(1.0f, 1.0f));

		// 뒷면
		positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
		positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
		positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
		positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
		colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
		colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
		colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
		colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
		normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
		normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
		normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
		normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
		coords.push_back(Vector2(0.0f, 1.0f));
		coords.push_back(Vector2(0.0f, 0.0f));
		coords.push_back(Vector2(1.0f, 0.0f));
		coords.push_back(Vector2(1.0f, 1.0f));

		// 왼쪽
		positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
		positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
		positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
		positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
		colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
		colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
		colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
		colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
		normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
		normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
		normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
		normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
		coords.push_back(Vector2(0.0f, 1.0f));
		coords.push_back(Vector2(0.0f, 0.0f));
		coords.push_back(Vector2(1.0f, 0.0f));
		coords.push_back(Vector2(1.0f, 1.0f));

		// 오른쪽
		positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
		positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
		positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
		positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
		colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
		colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
		colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
		colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
		normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
		normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
		normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
		normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
		coords.push_back(Vector2(0.0f, 1.0f));
		coords.push_back(Vector2(0.0f, 0.0f));
		coords.push_back(Vector2(1.0f, 0.0f));
		coords.push_back(Vector2(1.0f, 1.0f));

		MeshData meshData;

		for (size_t i = 0; i < positions.size(); i++) {
			Vertex v;
			v.position = positions[i];
			v.color = colors[i];
			v.normal = normals[i];
			v.uv = coords[i];
			meshData.vertices.push_back(v);
		}

		meshData.indices = {
			0,  1,  2,  0,  2,  3,  // 윗면
			4,  5,  6,  4,  6,  7,  // 아랫면
			8,  9,  10, 8,  10, 11, // 앞면
			12, 13, 14, 12, 14, 15, // 뒷면
			16, 17, 18, 16, 18, 19, // 왼쪽
			20, 21, 22, 20, 22, 23  // 오른쪽
		};

		return meshData;
	}

}



