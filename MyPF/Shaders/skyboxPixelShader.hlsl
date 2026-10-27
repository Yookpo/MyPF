TextureCube environmentMap : register(t0);
SamplerState linearSampler : register(s0);

struct SKYBOX_PS_INPUT
{
    float4 pos : SV_Position;
    float3 direction : TEXCOORD0;
};

float4 main(SKYBOX_PS_INPUT input) : SV_Target
{
    // 큐브맵 방향으로 샘플링
    float3 skyColor = environmentMap.Sample(linearSampler, input.direction).rgb;
    
    return float4(skyColor, 1.0);

}