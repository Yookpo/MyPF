#include "GeometryGenerator.h"

namespace My
{
	using namespace std;
	using namespace DirectX;
	using namespace DirectX::SimpleMath;

	MeshData GeometryGenerator::MakeTriangle()
	{
		vector<Vector3> positions;
		vector<Vector3> normals;
		vector<Vector2> coords;

		positions.push_back(Vector3(-1.0f, 0.0f, 0.0f));
		positions.push_back(Vector3(0.0f, 1.0f, 0.0f));
		positions.push_back(Vector3(1.0f, 0.0f, 0.0f));

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
			v.normal = normals[i];
			v.uv = coords[i];

			meshData.vertices.push_back(v);
		}
		meshData.indices = { 0, 1, 2 };

		return meshData;
	}

	MeshData GeometryGenerator::MakeCube()
	{
		vector<Vector3> positions;
		vector<Vector3> normals;
		vector<Vector2> coords;
		vector<Vector3> tangents;

		const float scale = 1.0f;

		// 윗면
		positions.push_back(Vector3(-0.5f, 0.5f, -0.5f) * scale);
		positions.push_back(Vector3(-0.5f, 0.5f, 0.5f) * scale);
		positions.push_back(Vector3(0.5f, 0.5f, 0.5f) * scale);
		positions.push_back(Vector3(0.5f, 0.5f, -0.5f) * scale);
		normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
		normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
		normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
		normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
		coords.push_back(Vector2(0.0f, 1.0f));
		coords.push_back(Vector2(0.0f, 0.0f));
		coords.push_back(Vector2(1.0f, 0.0f));
		coords.push_back(Vector2(1.0f, 1.0f));
		tangents.push_back(Vector3(1.0f, 0.0f, 0.0f));
		tangents.push_back(Vector3(1.0f, 0.0f, 0.0f));
		tangents.push_back(Vector3(1.0f, 0.0f, 0.0f));
		tangents.push_back(Vector3(1.0f, 0.0f, 0.0f));

		// 아랫면
		positions.push_back(Vector3(-0.5f, -0.5f, -0.5f) * scale);
		positions.push_back(Vector3(0.5f, -0.5f, -0.5f) * scale);
		positions.push_back(Vector3(0.5f, -0.5f, 0.5f) * scale);
		positions.push_back(Vector3(-0.5f, -0.5f, 0.5f) * scale);
		normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
		normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
		normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
		normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
		coords.push_back(Vector2(0.0f, 1.0f));
		coords.push_back(Vector2(0.0f, 0.0f));
		coords.push_back(Vector2(1.0f, 0.0f));
		coords.push_back(Vector2(1.0f, 1.0f));
		tangents.push_back(Vector3(0.0f, 0.0f, 1.0f));
		tangents.push_back(Vector3(0.0f, 0.0f, 1.0f));
		tangents.push_back(Vector3(0.0f, 0.0f, 1.0f));
		tangents.push_back(Vector3(0.0f, 0.0f, 1.0f));

		// 앞면
		positions.push_back(Vector3(-0.5f, -0.5f, -0.5f) * scale);
		positions.push_back(Vector3(-0.5f, 0.5f, -0.5f) * scale);
		positions.push_back(Vector3(0.5f, 0.5f, -0.5f) * scale);
		positions.push_back(Vector3(0.5f, -0.5f, -0.5f) * scale);
		normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
		normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
		normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
		normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
		coords.push_back(Vector2(0.0f, 1.0f));
		coords.push_back(Vector2(0.0f, 0.0f));
		coords.push_back(Vector2(1.0f, 0.0f));
		coords.push_back(Vector2(1.0f, 1.0f));
		tangents.push_back(Vector3(1.0f, 0.0f, 0.0f));
		tangents.push_back(Vector3(1.0f, 0.0f, 0.0f));
		tangents.push_back(Vector3(1.0f, 0.0f, 0.0f));
		tangents.push_back(Vector3(1.0f, 0.0f, 0.0f));

		// 뒷면
		positions.push_back(Vector3(-0.5f, -0.5f, 0.5f) * scale);
		positions.push_back(Vector3(0.5f, -0.5f, 0.5f) * scale);
		positions.push_back(Vector3(0.5f, 0.5f, 0.5f) * scale);
		positions.push_back(Vector3(-0.5f, 0.5f, 0.5f) * scale);
		normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
		normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
		normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
		normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
		coords.push_back(Vector2(1.0f, 1.0f));
		coords.push_back(Vector2(0.0f, 1.0f));
		coords.push_back(Vector2(0.0f, 0.0f));
		coords.push_back(Vector2(1.0f, 0.0f));
		tangents.push_back(Vector3(-1.0f, 0.0f, 0.0f));
		tangents.push_back(Vector3(-1.0f, 0.0f, 0.0f));
		tangents.push_back(Vector3(-1.0f, 0.0f, 0.0f));
		tangents.push_back(Vector3(-1.0f, 0.0f, 0.0f));

		// 왼쪽
		positions.push_back(Vector3(-0.5f, -0.5f, 0.5f) * scale);
		positions.push_back(Vector3(-0.5f, 0.5f, 0.5f) * scale);
		positions.push_back(Vector3(-0.5f, 0.5f, -0.5f) * scale);
		positions.push_back(Vector3(-0.5f, -0.5f, -0.5f) * scale);
		normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
		normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
		normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
		normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
		coords.push_back(Vector2(0.0f, 1.0f));
		coords.push_back(Vector2(0.0f, 0.0f));
		coords.push_back(Vector2(1.0f, 0.0f));
		coords.push_back(Vector2(1.0f, 1.0f));
		tangents.push_back(Vector3(0.0f, 0.0f, -1.0f));
		tangents.push_back(Vector3(0.0f, 0.0f, -1.0f));
		tangents.push_back(Vector3(0.0f, 0.0f, -1.0f));
		tangents.push_back(Vector3(0.0f, 0.0f, -1.0f));

		// 오른쪽
		positions.push_back(Vector3(0.5f, -0.5f, 0.5f) * scale);
		positions.push_back(Vector3(0.5f, -0.5f, -0.5f) * scale);
		positions.push_back(Vector3(0.5f, 0.5f, -0.5f) * scale);
		positions.push_back(Vector3(0.5f, 0.5f, 0.5f) * scale);
		normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
		normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
		normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
		normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
		coords.push_back(Vector2(1.0f, 1.0f));
		coords.push_back(Vector2(0.0f, 1.0f));
		coords.push_back(Vector2(0.0f, 0.0f));
		coords.push_back(Vector2(1.0f, 0.0f));
		tangents.push_back(Vector3(0.0f, 0.0f, 1.0f));
		tangents.push_back(Vector3(0.0f, 0.0f, 1.0f));
		tangents.push_back(Vector3(0.0f, 0.0f, 1.0f));
		tangents.push_back(Vector3(0.0f, 0.0f, 1.0f));

		MeshData meshData;

		for (size_t i = 0; i < positions.size(); i++)
		{
			Vertex v;
			v.position = positions[i];
			v.normal = normals[i];
			v.uv = coords[i];
			v.tangent = tangents[i];
			meshData.vertices.push_back(v);
		}

		meshData.indices = {
			0, 1, 2, 0, 2, 3,		// 윗면
			4, 5, 6, 4, 6, 7,		// 아랫면
			8, 9, 10, 8, 10, 11,	// 앞면
			12, 13, 14, 12, 14, 15, // 뒷면
			16, 17, 18, 16, 18, 19, // 왼쪽
			20, 21, 22, 20, 22, 23	// 오른쪽
		};

		return meshData;
	}

	MeshData GeometryGenerator::MakeSphere(float radius, int numSlices, int numStacks)
	{
		const float dTheta = -XM_2PI / float(numSlices);
		const float dPhi = -XM_PI / float(numStacks);

		MeshData meshData;

		for (int j = 0; j <= numStacks; j++)
		{
			// 맨 아래 점을 Z축으로 돌려 위도 j의 시작점을 만든다
			const Vector3 stackStartPoint =
				Vector3::Transform(Vector3(0.0f, -radius, 0.0f), Matrix::CreateRotationZ(dPhi * j));

			for (int i = 0; i <= numSlices; i++)
			{
				const Matrix sliceRotation = Matrix::CreateRotationY(dTheta * float(i));

				Vertex v;
				v.position = Vector3::Transform(stackStartPoint, sliceRotation);

				v.normal = v.position; // 중심이 원점이라 위치 방향이 곧 바깥 법선
				v.normal.Normalize();

				// j가 아래에서 위로 커지므로 v는 아래로 갈수록 커진다
				v.uv = Vector2(float(i) / numSlices, 1.0f - float(j) / numStacks);

				// 탄젠트 = u가 커지는 방향. 경도선 방향은 위도와 무관해서 적도의 (0, 0, −1)을 같은 회전으로 돌린다.
				v.tangent = Vector3::Transform(Vector3(0.0f, 0.0f, -1.0f), sliceRotation);

				meshData.vertices.push_back(v);
			}
		}

		for (int j = 0; j < numStacks; j++)
		{
			const uint32_t offset = static_cast<uint32_t>((numSlices + 1) * j);
			const uint32_t ring = static_cast<uint32_t>(numSlices + 1);

			for (uint32_t i = 0; i < static_cast<uint32_t>(numSlices); i++)
			{
				meshData.indices.push_back(offset + i);
				meshData.indices.push_back(offset + i + ring);
				meshData.indices.push_back(offset + i + 1 + ring);

				meshData.indices.push_back(offset + i);
				meshData.indices.push_back(offset + i + 1 + ring);
				meshData.indices.push_back(offset + i + 1);
			}
		}

		return meshData;
	}
} // namespace My