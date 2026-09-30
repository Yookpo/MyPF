#include "Common.hlsli"
#include "Lighting.hlsli"

Texture2D albedoTexture : register(t0);
SamplerState linearSampler : register(s0);

cbuffer MaterialConstantBuffer : register(b2)
{
    float3 baseColor;
    float roughness;
    float3 emissiveColor;
    float emissiveIntensity;
    float3 rimColor;
    float rimIntensity;
    float rimPower;
    float specular;
    float2 pad3;
}

float4 main(PS_INPUT input) : SV_TARGET
{
    IncidentLight dirLight = GetDirectionalLight();
    
    float3 normal = normalize(input.normal);
    float3 texColor = albedoTexture.Sample(linearSampler, input.uv).rgb;
    float3 surfaceColor = texColor * baseColor;
    float3 viewDir = normalize(cameraPos - input.posWorld);
    
    SurfaceData surfaceData;
    surfaceData.albedo = surfaceColor;
    surfaceData.normal = normal;
    surfaceData.roughness = roughness;
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

    float rimFactor = pow(1.0 - saturate(dot(normal, viewDir)), rimPower);
    float3 rim = rimColor * rimIntensity * rimFactor;
    
    // 합성
    float3 finalColor = ambientColor + directLighting + emissive + rim;
    
    return float4(finalColor, 1.0f);
}

