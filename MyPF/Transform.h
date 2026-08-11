#pragma once
#include <directxtk/SimpleMath.h>

namespace My
{
	using DirectX::SimpleMath::Vector3;
	using DirectX::SimpleMath::Matrix;


	class Transform
	{
	public:
		Transform();
		const Vector3& GetPosition() const { return m_position; }
		const Vector3& GetRotation() const { return m_rotation; }
		const Vector3& GetScale() const { return m_scale; }
		void SetPosition(const Vector3& newPos) { m_position = newPos; }
		void SetRotation(const Vector3& newRot) { m_rotation = newRot; }
		void SetScale(const Vector3& newScale) { m_scale = newScale; }
		Matrix GetWorldMatrix() const;


	private:
		Vector3 m_position;
		Vector3 m_rotation;
		Vector3 m_scale;
	};

}


