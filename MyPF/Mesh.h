#pragma once
#include "MeshData.h"
#include <d3d11.h>
#include <wrl.h>

namespace My
{
	using Microsoft::WRL::ComPtr;

	class Mesh
	{
	public:
		Mesh() = default;
		Mesh(const Mesh&) = delete;
		Mesh& operator = (const Mesh&) = delete;

		bool Initialize(ID3D11Device*, const MeshData&);

		ID3D11Buffer* GetVertexBuffer() const { return m_vertexBuffer.Get(); }
		ID3D11Buffer* GetIndexBuffer() const { return m_indexBuffer.Get(); }
		UINT GetIndexCount() const { return m_indexCount; }

	private:
		// meshes
		ComPtr<ID3D11Buffer> m_vertexBuffer;
		ComPtr<ID3D11Buffer> m_indexBuffer;
		UINT m_indexCount = 0;
	};

}




