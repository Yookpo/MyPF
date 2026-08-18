Texture2D albedoTexture : register(t0);
SamplerState linearSampler : register(s0);

cbuffer LightConstantData : register(b0)
{
    float3 direction;
    float intensity;
    float3 color;
    float pad;
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
    float4 position : SV_POSITION;
    float3 color : COLOR0;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
};


float4 main(PS_INPUT input) : SV_TARGET
{
    float3 normal = normalize(input.normal);
    float diffuse = saturate(dot(-direction, normal));
    
    float3 albedo = albedoTexture.Sample(linearSampler, input.uv).rgb;
    albedo *= baseColor;
    
    float3 finalColor = albedo * color * intensity * diffuse;
    
    return float4(finalColor, 1.0f);
}

