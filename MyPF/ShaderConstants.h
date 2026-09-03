#pragma once
#include <directxtk/SimpleMath.h>
#include <cstdint>
#include "PointLight.h"

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

	struct PointLightConstantData
	{
		Vector3		  position;	 // 12
		float		  range;	 // 4
		Vector3		  color;	 // 12
		float		  intensity; // 4
		std::uint32_t isEnabled; // 4
		Vector3		  pad;		 // 12
	};

	struct LightConstantData
	{
		// Directional Light 데이터
		Vector3 direction;		 // 12
		float	intensity;		 // 4
		Vector3 color;			 // 12
		float	ambientStrength; // 4
		// Point Light
		std::uint32_t		   pointLightCount; // 4
		Vector3				   pad;				// 12
		PointLightConstantData pointLights[MaxPointLightCount];
	};

	struct MaterialConstantData
	{
		Vector3 baseColor;		   // 12
		float	pad;			   // 4
		Vector3 emissiveColor;	   // 12
		float	emissiveIntensity; // 4
	};
} // namespace My
