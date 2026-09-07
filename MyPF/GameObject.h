#pragma once
#include <string>
#include "Transform.h"
#include "MeshComponent.h"
#include "ModelComponent.h"
#include "PointLightComponent.h"

namespace My
{
	class GameObject
	{
	public:
		GameObject(const std::string& name);

		const std::string& GetName() const { return m_name; }
		void			   SetName(const std::string& name) { m_name = name; }

		Transform&		 GetTransform() { return m_transform; }		  // 수정 가능
		const Transform& GetTransform() const { return m_transform; } // 읽기전용

		MeshComponent&		 GetMeshComponent() { return m_meshComponent; }		  // 수정 가능
		const MeshComponent& GetMeshComponent() const { return m_meshComponent; } // 읽기전용

		ModelComponent&		  GetModelComponent() { return m_modelComponent; }		 // 수정 가능
		const ModelComponent& GetModelComponent() const { return m_modelComponent; } // 읽기전용

		PointLightComponent& AddPointLightComponent()
		{
			m_hasPointLightComponent = true;
			return m_pointLightComponent;
		}
		PointLightComponent&	   GetPointLightComponent() { return m_pointLightComponent; }		// 수정 가능
		const PointLightComponent& GetPointLightComponent() const { return m_pointLightComponent; } // 읽기전용
		bool					   HasPointLightComponent() const { return m_hasPointLightComponent; }

	private:
		std::string			m_name;
		Transform			m_transform;
		MeshComponent		m_meshComponent;
		ModelComponent		m_modelComponent;
		PointLightComponent m_pointLightComponent;
		bool				m_hasPointLightComponent{ false };
	};

} // namespace My
