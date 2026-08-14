#pragma once
#include "Transform.h"
#include "MeshComponent.h"
#include <string>

namespace My
{
	class GameObject
	{
	public:
		GameObject(const std::string& name);
		const std::string& GetName() const { return m_name; }
		void SetName(const std::string& name) { m_name = name; }
		Transform& GetTransform() { return m_transform; }	// 수정 가능
		const Transform& GetTransform() const { return m_transform; }	// 읽기전용
		MeshComponent& GetMeshComponent() { return m_meshComponent; }	// 수정 가능
		const MeshComponent& GetMeshComponent() const { return m_meshComponent; }	// 읽기전용

	private:
		std::string m_name;
		Transform m_transform;
		MeshComponent m_meshComponent;
	};


}



