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

		positions.push_back(Vector3(-1.0f, 0.0f, 0.0f));
		positions.push_back(Vector3(0.0f, 1.0f, 0.0f));
		positions.push_back(Vector3(1.0f, 0.0f, 0.0f));

		colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
		colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
		colors.push_back(Vector3(1.0f, 0.0f, 0.0f));

		MeshData meshData;

		for (int i = 0; i < 3; i++)
		{
			Vertex v;
			v.position = positions[i];
			v.normal = colors[i];

			meshData.vertices.push_back(v);
		}
		meshData.indices = {
			0,1,2
		};

		return meshData;
	}

}

