#include "Common.hlsli"

struct SKYBOX_PS_INPUT
{
    float4 pos : SV_Position;
    float3 direction : TEXCOORD0;   // 큐브맵 샘플 방향 = 큐브의 로컬 위치
};

SKYBOX_PS_INPUT main(VS_INPUT input)
{
    SKYBOX_PS_INPUT output;
    
    // w = 0 : view의 이동을 지우고 회전만 남긴다. 걸어도 하늘은 무한히 멀리 있다고 가정
    float4 posView = mul(float4(input.pos, 0.0), view);
    float4 posClip = mul(float4(posView.xyz, 1.0), projection);
    
    output.pos = posClip.xyww;
    output.direction = input.pos;
    
    return output;
}