#pragma once

namespace My
{
	class Mesh;
	class Material;

	class MeshComponent
	{
	public:
		MeshComponent() : m_mesh(nullptr), m_material(nullptr) {}
		void SetMesh(const Mesh* newMesh) { m_mesh = newMesh; }
		const Mesh* GetMesh() const { return m_mesh; }
		bool HasMesh() const { return m_mesh; }

		void SetMaterial(Material* newMat) { m_material = newMat; }
		const Material* GetMaterial() const { return m_material; }	// 읽기 전용
		Material* GetMaterial() { return m_material; }	// 수정 가능
		bool HasMaterial() const { return m_material; }

	private:
		const Mesh* m_mesh;			// 오브젝트가 사용할 메쉬 저장
		Material* m_material;		// 오브젝트가 사용할 머터리얼
	};
}


