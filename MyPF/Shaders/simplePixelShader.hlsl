#include "Common.hlsli"
#include "Lighting.hlsli"

Texture2D albedoTexture : register(t0);
Texture2D roughnessTexture : register(t1);
Texture2D normalTexture : register(t2);

SamplerState linearSampler : register(s0);

cbuffer MaterialConstantBuffer : register(b2)
{
    float3 baseColor;
    float roughnessFactor;
    float3 emissiveColor;
    float emissiveIntensity;
    float3 rimColor;
    float rimIntensity;
    float rimPower;
    float specular;
    float2 uvScale;
}

float4 main(PS_INPUT input) : SV_TARGET
{
    IncidentLight dirLight = GetDirectionalLight();
    
    float3 normal = normalize(input.normal);
    float2 tiledUV = input.uv * uvScale;
    
    float3 texColor = albedoTexture.Sample(linearSampler, tiledUV).rgb;
    float roughnessSample = roughnessTexture.Sample(linearSampler, tiledUV).r;
    
    float3 normalTS = normalTexture.Sample(linearSampler, tiledUV).rgb;
    normalTS = 2.0 * normalTS - 1.0;

    float3 N = normal;
    float3 T = normalize(input.tangentWorld - dot(input.tangentWorld, N) * N);
    float3 B = cross(N, T);
    float3x3 TBN = float3x3(T, B, N);
    
    float3 surfaceColor = texColor * baseColor;
    float3 viewDir = normalize(cameraPos - input.posWorld);
    
    SurfaceData surfaceData;
    surfaceData.albedo = surfaceColor;
    surfaceData.normal = normalize(mul(normalTS, TBN));
    surfaceData.roughness = roughnessFactor * roughnessSample;
    surfaceData.specular = specular;
    
    
    // 직접광 누적 변수
 
    // 1. 방향광 결과로 시작
    float3 directLighting = ComputeDirectLighting(surfaceData, dirLight, viewDir);
    
    // 2. 누적 -> 점광원
    for (uint i = 0; i < pointLightCount; i++)
    {
        IncidentLight pointLight = GetPointLight(i, input.posWorld);
        directLighting += ComputeDirectLighting(surfaceData, pointLight, viewDir);
    }
    
    // 3. 간접광
    float3 ambientColor = surfaceData.albedo * ambientStrength;
    
    // 4. 연출 항 (물리 조명은 아님)
    float3 emissive = emissiveColor * emissiveIntensity;

    float rimFactor = pow(1.0 - saturate(dot(surfaceData.normal, viewDir)), rimPower);
    float3 rim = rimColor * rimIntensity * rimFactor;
    
    // 합성
    float3 finalColor = ambientColor + directLighting + emissive + rim;
    
    return float4(finalColor, 1.0f);
}

