#include "Common.hlsli"
#include "Lighting.hlsli"

Texture2D albedoTexture : register(t0);
Texture2D roughnessTexture : register(t1);
Texture2D normalTexture : register(t2);
Texture2D puddleMaskTexture : register(t3);
TextureCube environmentMap : register(t4);

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

// 웅덩이(puddleMask = 1)의 표면 값. Material cbuffer가 꽉 차서 셰이더 상수로 둔다
// 물은 거의 거울(0)이다. Blinn-Phong 때는 0.1이면 하이라이트가 점 하나로 줄어 0.15였지만,
// GGX(10-2')는 꼬리가 길어 0.1에서도 젖은 반사 줄이 남아 0.1로 낮췄다(N·H = 1에서 D ≈ 3200)
static const float PUDDLE_ROUGHNESS = 0.1;
// 선형 배율(BaseColor와 달리 sRGB 아님). 간접광 반사가 없어(10-5 전) 0.5면 물이 아니라 검은 얼룩처럼 보인다
static const float PUDDLE_ALBEDO_SCALE = 0.75;


float4 main(PS_INPUT input) : SV_TARGET
{
    IncidentLight dirLight = GetDirectionalLight();
    
    float3 normal = normalize(input.normal);
    float2 tiledUV = input.uv * uvScale;
    
    float3 texColor = albedoTexture.Sample(linearSampler, tiledUV).rgb;
    float roughnessSample = roughnessTexture.Sample(linearSampler, tiledUV).r;
    float puddleMaskSample = puddleMaskTexture.Sample(linearSampler, input.uv).r;
    
    float3 normalTS = normalTexture.Sample(linearSampler, tiledUV).rgb;
    normalTS = 2.0 * normalTS - 1.0;
    normalTS = lerp(normalTS, float3(0.0, 0.0, 1.0), puddleMaskSample);

    float3 N = normal;
    float3 T = normalize(input.tangentWorld - dot(input.tangentWorld, N) * N);
    float3 B = cross(N, T);
    float3x3 TBN = float3x3(T, B, N);
    
    float3 surfaceColor = texColor * baseColor;
    float3 viewDir = normalize(cameraPos - input.posWorld);
    
    SurfaceData surfaceData;
    surfaceData.albedo = lerp(surfaceColor, surfaceColor * PUDDLE_ALBEDO_SCALE, puddleMaskSample);
    surfaceData.normal = normalize(mul(normalTS, TBN));
    surfaceData.roughness = lerp(roughnessFactor * roughnessSample, PUDDLE_ROUGHNESS, puddleMaskSample);
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
    
    float3 reflectDir = reflect(-viewDir, surfaceData.normal);
    float3 environmentRadiance = SampleEnvironmentRadiance(environmentMap, linearSampler, reflectDir, surfaceData.roughness);
    float3 environmentSpecular = ComputeEnvironmentSpecular(surfaceData, viewDir, environmentRadiance);
   
    // 4. 연출 항 (물리 조명은 아님)
    float3 emissive = emissiveColor * emissiveIntensity;

    float rimFactor = pow(1.0 - saturate(dot(surfaceData.normal, viewDir)), rimPower);
    float3 rim = rimColor * rimIntensity * rimFactor;
    
    // 합성
    float3 finalColor = ambientColor + environmentSpecular + directLighting + emissive + rim;
    
    return float4(finalColor, 1.0f);
}

