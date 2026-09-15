Texture2D sourceTexture : register(t0);
SamplerState linearSampler : register(s0);

struct Output
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};


float4 main(Output input) : SV_Target
{
    return float4(sourceTexture.Sample(linearSampler, input.uv).rgb, 1.0f);
}