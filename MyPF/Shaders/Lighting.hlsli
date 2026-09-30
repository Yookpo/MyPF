#ifndef LIGHTING_HLSLI
#define LIGHTING_HLSLI

#include "Common.hlsli"

#define NUM_POINT_LIGHTS 8

struct PointLight
{
    float3 position;
    float range;
    float3 color;
    float intensity;
    uint isEnabled;
    float3 pad;
};

// 표면의 한 점에 도착하는 빛
// 빛 종류(Directional/Point/Spot)과 무관
struct IncidentLight
{
    float3 direction; // 표면 -> 빛, unit vec
    float3 radiance; // 도착한 빛의 양 = color * intensity * 감쇠
};

// 쉐이딩할 표면 한 점의 속성
struct SurfaceData
{
    float3 normal; // 월드 공간, unit vec
    float3 albedo; // 선형 공간 표면 색 = 텍스처 * baseColor
    float roughness; // 거칠기 [0.05, 1], BRDF에서 α = roughness²로 변환
    float specular; // 비금속 반사량 [0, 1] , F0 = 0.08 * specular (0.5 → 0.04)
};

cbuffer LightConstantBuffer : register(b0)
{
    float3 dirLightDirection;
    float dirLightIntensity;
    float3 dirLightColor;
    float ambientStrength;
    uint pointLightCount;
    float3 lightPad;
    PointLight pointLights[NUM_POINT_LIGHTS];
}

float CalcAttenuation(float lightDistance, float range)
{
    // Linear falloff
    if (range <= 0)
    {
        return 0;
    }
    
    lightDistance /= range;
    
    float aten = saturate(lightDistance);
    
    aten = (1 - aten);
    
    return (aten * aten);
}

// 어느 표면에서나 같은 값
IncidentLight GetDirectionalLight()
{
    IncidentLight directionalLight;
    
    directionalLight.direction = -dirLightDirection;
    directionalLight.direction = normalize(directionalLight.direction);

    directionalLight.radiance = dirLightColor * dirLightIntensity;

    return directionalLight;
}

// index번째 Point Light가 posWorld에 도착시키는 빛
// 꺼져 있거나 범위 밖이면 radiance = 0 (direction은 안전한 unit Vec)
IncidentLight GetPointLight(uint index, float3 posWorld)
{
    PointLight pointLight = pointLights[index];
    IncidentLight outPointLight;
    
    outPointLight.direction = float3(0, 1, 0);
    outPointLight.radiance = float3(0, 0, 0);
    
    if (pointLight.isEnabled == 0)
    {
        return outPointLight;
    }
    
    float3 toLight = pointLight.position - posWorld;
    
    // 쉐이딩할 지점부터 조명까지의 거리 
    float d = length(toLight);
    
    // 너무 멀면 조명 적용 X
    if (d >= pointLight.range || d <= 0.0001f)
    {
        return outPointLight;
    }

    outPointLight.direction = normalize(toLight);
    outPointLight.radiance = pointLight.color * pointLight.intensity * CalcAttenuation(d, pointLight.range);
    
    return outPointLight;
}

// 정규화된 Blinn-Phong: pow(N·H, n) × (n+8)/8. roughness가 바뀌어도 총 반사량 유지
float SpecularBlinnPhong(float NdotH, float roughness)
{
    float alpha = pow(roughness, 2);
    float n = 2 / pow(alpha, 2) - 2;
    n = max(n, 1);

    return pow(NdotH, n) * (n + 8) / 8;
}

// 빛 하나가 표면에 만드는 직접광. 모든 빛 종류가 이 함수를 지난다
float3 ComputeDirectLighting(SurfaceData surface, IncidentLight light, float3 viewDir)
{
    float NdotL = saturate(dot(surface.normal, light.direction));
    // 빛이 표면 뒤나 수평에서 오면 계산 생략
    if (NdotL <= 1e-8)
    {
        return float3(0, 0, 0);
    }
    
    float3 halfVec = light.direction + viewDir;
    // L + v = 0이면, normalize에러(NaN)
    if (dot(halfVec, halfVec) <= 1e-8)
    {
        return float3(0, 0, 0);
    }
    halfVec = normalize(halfVec);
    
    float NdotH = saturate(dot(surface.normal, halfVec));
    float f0 = 0.08 * surface.specular;
    
    float specularTerm = SpecularBlinnPhong(NdotH, surface.roughness) * f0;
    
    float3 directColor = (surface.albedo + specularTerm) * light.radiance * NdotL;
    
    return directColor;
}

#endif