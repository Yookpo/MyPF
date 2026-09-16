#include "PostProcess.hlsli"

Texture2D hdrTexture : register(t0);
SamplerState linearSampler : register(s0);

struct Output
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

float4 main(Output input) : SV_Target
{
    float3 color = hdrTexture.Sample(linearSampler, input.uv).rgb;
    
    color *= exposure;
    
    const float lum = dot(color, float3(0.2126, 0.7152, 0.0722));
    
    if (lum < threshold)
    {
        return float4(0.0, 0.0, 0.0, 0.0);
    }
    else
    {
        return float4(color, 1.0);

    }
    
}