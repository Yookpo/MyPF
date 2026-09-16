#include "PostProcess.hlsli"
Texture2D g_texture0 : register(t0);
SamplerState g_sampler : register(s0);

struct Output
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

static const float weights[5] = { 0.0545, 0.2442, 0.4026, 0.2442, 0.0545 };

float4 main(Output input) : SV_TARGET
{
    float3 color = float3(0.0, 0.0, 0.0);
    
    [unroll]
    for (int i = -2; i <= 2; i++)
    {
        float2 offset = float2(float(i) * dx, 0.0);
        color += weights[i + 2] * g_texture0.Sample(g_sampler, input.uv + offset).rgb;
    }
    
    return float4(color, 1.0);
}