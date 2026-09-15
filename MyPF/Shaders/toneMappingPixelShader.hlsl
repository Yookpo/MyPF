Texture2D hdrTexture : register(t0);
SamplerState linearSampler : register(s0);

cbuffer PostProcessConstantData : register(b0)
{
    float exposure;
    float3 pad;
}

struct Output
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};


float4 main(Output input) : SV_Target
{
    // UV로 HDR 텍스처를 샘플링해서 rgb만 꺼낸다. 알파는 쓰지 않는다.
    float3 color = hdrTexture.Sample(linearSampler, input.uv).rgb;
    
    // 카메라가 받아들이는 빛의 양
    color *= exposure;
    
    // Reinhard를 적용한다: 결과 = 색 / (1 + 색). HLSL은 float3에 대해 이 계산을 채널마다 따로 해준다. 반복문이 필요 없다.
    color = color / (1 + color);
    // 결과 rgb에 알파 1을 붙여 반환한다.
    return float4(color, 1.0f);

}