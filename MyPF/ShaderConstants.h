#pragma once
#include <directxtk/SimpleMath.h>

namespace My
{
	using DirectX::SimpleMath::Matrix;
	using DirectX::SimpleMath::Vector3;

	struct ObjectConstantData
	{
		Matrix model;
		Matrix invTranspose;
	};

	struct CameraConstantData
	{
		Matrix view;
		Matrix projection;
	};

	struct LightConstantData
	{
		Vector3 direction;		 // 12
		float	intensity;		 // 4
		Vector3 color;			 // 12
		float	ambientStrength; // 4
	};

	struct MaterialConstantData
	{
		Vector3 baseColor; // 12
		float	pad;	   // 4
	};
} // namespace My
