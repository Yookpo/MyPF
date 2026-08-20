#pragma once
#include <cstdint>
#include "MeshData.h"
#include "BufferHandle.h"

namespace My
{
	class GraphicsResourceManager;

	class Mesh
	{
	public:
		Mesh() = default;
		Mesh(const Mesh&) = delete;
		Mesh& operator = (const Mesh&) = delete;

		bool Initialize(GraphicsResourceManager&, const MeshData&);

		BufferHandle GetVertexBufferHandle() const { return m_vertexBufferHandle; }
		BufferHandle GetIndexBufferHandle() const { return m_indexBufferHandle; }

		uint32_t GetVertexStride() const { return m_vertexStride; }
		uint32_t GetIndexCount() const { return m_indexCount; }

	private:
		// meshes
		BufferHandle m_vertexBufferHandle;
		BufferHandle m_indexBufferHandle;

		uint32_t m_vertexStride = sizeof(Vertex);
		uint32_t m_indexCount = 0;
	};

}




