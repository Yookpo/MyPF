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
#endif