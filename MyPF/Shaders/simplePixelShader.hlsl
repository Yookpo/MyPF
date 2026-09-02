#include "Lighting.hlsli"
Texture2D albedoTexture : register(t0);
SamplerState linearSampler : register(s0);

cbuffer LightConstantData : register(b0)
{
    float3 direction;
    float intensity;
    float3 color;
    float ambientStrength;
    uint pointLightCount;
    float3 pad;
    PointLight pointLights[NUM_POINT_LIGHTS];
}

cbuffer MaterialConstantData : register(b1)
{
    float3 baseColor;
    float pad2;
}

struct VS_INPUT
{
    float3 position : POSITION0;
    float3 color : COLOR0;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 posWorld : TEXCOORD1; // 조명 계산용
    float3 color : COLOR0;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
};


float4 main(PS_INPUT input) : SV_TARGET
{
    float3 normal = normalize(input.normal);
    float diffuse = saturate(dot(-direction, normal)); // directional Light의 diffuseColor
    
    float3 albedo = albedoTexture.Sample(linearSampler, input.uv).rgb;
    float3 surfaceColor = albedo * baseColor;
    
    float3 ambientColor = surfaceColor * ambientStrength;
    float3 diffuseColor = surfaceColor * color * intensity * diffuse;
    
    float3 pointLightColor = float3(0, 0, 0);
    for (uint i = 0; i < pointLightCount; i++)
    {
        pointLightColor += ComputePointLight(pointLights[i], input.posWorld, normal, surfaceColor);
    }
    
    float3 finalColor = ambientColor + diffuseColor + pointLightColor;
    
    return float4(saturate(finalColor), 1.0f);
}

