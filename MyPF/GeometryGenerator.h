#pragma once
#include "MeshData.h"

namespace My
{
	class GeometryGenerator
	{
	public:
		static MeshData MakeTriangle();
		static MeshData MakeCube();
		static MeshData MakeSphere(float radius, int numSlices, int numStacks);

		// 필요한 지형데이터 입력함수 작성
	};
} // namespace My
