#include "Mesh.h"
#include "GraphicsResourceManager.h"

namespace My
{
	bool Mesh::Initialize(GraphicsResourceManager& resourceManager, const MeshData& meshData)
	{
		BufferHandle vertexHandle, indexHandle;

		vertexHandle = resourceManager.CreateVertexBuffer(meshData.vertices);
		if (!vertexHandle.IsValid())
		{
			return false;
		}

		m_vertexBufferHandle = vertexHandle;

		indexHandle = resourceManager.CreateIndexBuffer(meshData.indices);
		if (!indexHandle.IsValid())
		{
			return false;
		}

		m_indexBufferHandle = indexHandle;
		m_indexCount = uint32_t(meshData.indices.size());

		return true;
	}

}



