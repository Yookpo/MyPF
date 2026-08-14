#pragma once

namespace My
{
	class Mesh;

	class MeshComponent
	{
	public:
		MeshComponent() : m_mesh(nullptr) {}
		void SetMesh(const Mesh* newMesh) { m_mesh = newMesh; }
		const Mesh* GetMesh() const { return m_mesh; }
		bool HasMesh() const { return m_mesh; }


	private:
		const Mesh* m_mesh;	// 오브젝트가 사용할 메쉬 저장
	};
}


