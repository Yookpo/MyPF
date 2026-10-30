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

// roughness가 0이면 D_GGX의 N·H = 1 값 1/(π·α²)가 무한대로 간다. BRDF 입구(ComputeDirectLighting)에서 한 번 막는다
static const float MIN_PERCEPTUAL_ROUGHNESS = 0.05;

cbuffer LightConstantBuffer : register(b0)
{
    float3 dirLightDirection;
    float dirLightIntensity;
    float3 dirLightColor;
    float ambientStrength;
    uint pointLightCount;
    float environmentIntensity;
    float2 lightPad;
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

// GGX(Trowbridge-Reitz) 미세면 분포. a2 = α² = roughness⁴
// diffuse도 albedo / π라 /π는 정석 규약이다(2026-10-27 π 규약 전환)
// 분모는 N·H ≤ 1에서 최솟값이 a2(> 0)라 0으로 나눌 일이 없다
float D_GGX(float a2, float NdotH)
{
    float denom = (NdotH * a2 - NdotH) * NdotH + 1;

    return a2 / (PI * denom * denom);
}

// Smith 가시성 Vis = G / (4·N·L·N·V). 높이 상관 Smith의 근사
// N·V = 0이어도 분모가 N·L × a(> 0)로 남아 0으로 나눌 일이 없다
float Vis_SmithJointApprox(float a2, float NdotV, float NdotL)
{
    float a = sqrt(a2);
    float visSmithV = NdotL * (NdotV * (1.0 - a) + a);
    float visSmithL = NdotV * (NdotL * (1.0 - a) + a);

    return 0.5 / (visSmithV + visSmithL);
}

// Schlick 근사 Fresnel, 미세면(H)에 빛이 스치듯 들어올수록 반사율이 F0 -> 1로 커진다.
// 직접광은 cosθ로 V·H를, 간접광/IBL은 N·V
float F_Schlick(float f0, float cosTheta)
{
    float oneMinusCos = 1.0 - cosTheta;
    float fresnelWeight = (oneMinusCos * oneMinusCos * oneMinusCos * oneMinusCos * oneMinusCos);
    float reflectance = f0 + (1.0 - f0) * fresnelWeight;
    
    return reflectance;
}

// GGX specular의 D·Vis (F 제외)
// Unreal SpecularGGX와 달리 F를 곱하지 않는다: diffuse가 (1 − F)로 F를 함께 쓰므로 호출부에서 한 번 곱한다
float SpecularGGX(float roughness, float NdotH, float NdotV, float NdotL)
{
    float a2 = roughness * roughness * roughness * roughness;
    float D = D_GGX(a2, NdotH);
    float Vis = Vis_SmithJointApprox(a2, NdotV, NdotL);

    return D * Vis;
}

// 비금속 Specular를 정면 반사율 F0로
float DielectricSpecularToF0(float specular)
{
    return 0.08 * specular;
}

// roughness를 큐브맵 LOD로
float ComputeEnvironmentLOD(float roughness, uint mipCount)
{
    return roughness * (float(mipCount) - 1.0);
}

// 빛 쪽: 반사 방향에서 들어오는 환경 radiance. 거칠 수록 흐린 밉을 읽음
float3 SampleEnvironmentRadiance(TextureCube cubeMap, SamplerState cubeSampler, float3 direction, float roughness)
{
    uint width, height, mipCount;
    cubeMap.GetDimensions(0, width, height, mipCount);

    float lod = ComputeEnvironmentLOD(roughness, mipCount);

    return cubeMap.SampleLevel(cubeSampler, direction, lod).rgb * environmentIntensity;
}

// 환경광 Fresnel + 거칠기 감쇠
float EnvBRDFApprox(float f0, float roughness, float NdotV)
{
    const float4 c0 = float4(-1.0, -0.0275, -0.572, 0.022);
    const float4 c1 = float4(1.0, 0.0425, 1.04, -0.04);
    float4 r = roughness * c0 + c1;
    float a004 = min(r.x * r.x, exp2(-9.28 * NdotV)) * r.x + r.y;
    float2 AB = float2(-1.04, 1.04) * a004 + r.zw;
    
    return f0 * AB.x + AB.y;
}

// 표면 쪽: 들어온 환경 radiance 중 반사되는 몫
float3 ComputeEnvironmentSpecular(SurfaceData surface, float3 viewDir, float3 environmentRadiance)
{
    float NdotV = saturate(abs(dot(surface.normal, viewDir)) + 1e-5);
    float f0 = DielectricSpecularToF0(surface.specular);
    
    float F = EnvBRDFApprox(f0, surface.roughness, NdotV);

    return environmentRadiance * F;
}

// 빛 하나가 표면에 만드는 직접광. 모든 빛 종류가 이 함수를 지난다
float3 ComputeDirectLighting(SurfaceData surface, IncidentLight light, float3 viewDir)
{
    surface.roughness = max(surface.roughness, MIN_PERCEPTUAL_ROUGHNESS);
    
    float NdotL = saturate(dot(surface.normal, light.direction));
    // 빛이 표면 뒤나 수평에서 오면 계산 생략
    if (NdotL <= 1e-8)
    {
        return float3(0, 0, 0);
    }
    
    float NdotV = saturate(abs(dot(surface.normal, viewDir)) + 1e-5);
    
    float3 halfVec = light.direction + viewDir;
    // L + v = 0이면, normalize에러(NaN)
    if (dot(halfVec, halfVec) <= 1e-8)
    {
        return float3(0, 0, 0);
    }
    halfVec = normalize(halfVec);
    
    float NdotH = saturate(dot(surface.normal, halfVec));
    float VdotH = saturate(dot(viewDir, halfVec));
    
    float f0 = DielectricSpecularToF0(surface.specular);
    float F = F_Schlick(f0, VdotH);
    
    float specularDV = SpecularGGX(surface.roughness, NdotH, NdotV, NdotL);
    
    float specularTerm = specularDV * F;
 
    float diffuseWeight = 1 - F;
    float3 diffuseTerm = surface.albedo * diffuseWeight / PI;

    float3 directColor = (diffuseTerm + specularTerm) * light.radiance * NdotL;
    
    return directColor;
}

#endif