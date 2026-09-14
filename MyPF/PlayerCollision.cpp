#include "PlayerCollision.h"
#include <algorithm> // for clamp

namespace My
{
	Vector3 PlayerCollision::Resolve(
		const Vector3& desiredPosition, const std::vector<DirectX::BoundingBox>& colliders, float radius)
	{
		Vector3 pos = desiredPosition;

		for (const auto& box : colliders)
		{
			pos = PushOutOfBox(pos, box, radius);
		}

		return pos;
	}

	Vector3 PlayerCollision::PushOutOfBox(const Vector3& position, const DirectX::BoundingBox& box, float radius)
	{
		// step1 - 박스에서 플레이어와 가장 가까운 점 찾기 (clamp)
		// 플레이어는 XZ평면만 움직임 (점프 적용 없고, y고정)
		float clampedX = std::clamp(position.x, box.Center.x - box.Extents.x, box.Center.x + box.Extents.x);
		float clampedZ = std::clamp(position.z, box.Center.z - box.Extents.z, box.Center.z + box.Extents.z);

		// step 2 - 그 점과 플레이어 사이 거리 재기
		Vector3 diff = position - Vector3(clampedX, position.y, clampedZ);
		float	diffLength = diff.Length();

		// step 3 - radius보다 가까우면 밀어내기
		// diffLength가 radius보다 가까우면 침투상태이므로 밀어내야한다
		if (diffLength < radius)
		{
			// 방향 정규화
			diff.Normalize();

			// 밀어낼 거리
			const float offSet = radius - diffLength;

			// 보정된 위치 :: playerPos + 방향 * 밀어낼 거리
			return position + diff * offSet;
		}

		return position;
	}
} // namespace My
