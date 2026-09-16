Texture2D hdrTexture : register(t0);
SamplerState linearSampler : register(s0);

cbuffer PostProcessConstantData : register(b0)
{
    float exposure;
    float threshold;
    uint toneMapper;
    float pad;
}

struct Output
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

float3 Reinhard(float3 x)
{
    return x / (1 + x);
}

float3 ACESFilm(float3 x)
{
    return saturate(x * (2.51 * x + 0.03) / (x * (2.43 * x + 0.59) + 0.14));
}

float4 main(Output input) : SV_Target
{
    // UV로 HDR 텍스처를 샘플링해서 rgb만 꺼낸다. 알파는 쓰지 않는다.
    float3 color = hdrTexture.Sample(linearSampler, input.uv).rgb;
    
    // 카메라가 받아들이는 빛의 양
    color *= exposure;
    
    // ToneMapper 값과 순서가 같아야 한다 (0: Reinhard, 1: ACES)
    if (toneMapper == 0)
    {
        color = Reinhard(color);
    }
    else
    {
        color = ACESFilm(color);
    }
    
    // 감마 인코드
    return float4(pow(color, 1 / 2.2), 1.0f);

}