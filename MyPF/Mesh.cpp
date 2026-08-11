#include "Mesh.h"
#include "D3D11Utils.h"

namespace My
{
	bool Mesh::Initialize(ID3D11Device* device, const MeshData& meshData)
	{
		if (!D3D11Utils::CreateVertexBuffer(device, meshData.vertices,
			m_vertexBuffer))
		{
			return false;
		}

		if (!D3D11Utils::CreateIndexBuffer(device, meshData.indices, m_indexBuffer))
		{
			return false;
		}

		m_indexCount = UINT(meshData.indices.size());

		return true;
	}

}

