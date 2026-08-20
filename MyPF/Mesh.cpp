#include "Mesh.h"
#include "GraphicsResourceManager.h"

namespace My
{
	bool Mesh::Initialize(GraphicsResourceManager& resourceManager, const MeshData& meshData)
	{
		m_vertexBufferHandle = resourceManager.CreateVertexBuffer(meshData.vertices);
		if (!m_vertexBufferHandle.IsValid())
		{
			return false;
		}

		m_indexBufferHandle = resourceManager.CreateIndexBuffer(meshData.indices);
		if (!m_indexBufferHandle.IsValid())
		{
			return false;
		}

		m_indexCount = uint32_t(meshData.indices.size());

		return true;
	}

}



