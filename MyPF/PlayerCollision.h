#pragma once
#include <directxtk/SimpleMath.h>
#include <DirectXCollision.h>
#include <vector>

namespace My
{
	using DirectX::SimpleMath::Vector3;

	class PlayerCollision
	{
	public:
		static Vector3 Resolve(
			const Vector3& desiredPosition, const std::vector<DirectX::BoundingBox>& colliders, float radius);

	private:
		static Vector3 PushOutOfBox(const Vector3& position, const DirectX::BoundingBox& box, float radius);
	};
} // namespace My