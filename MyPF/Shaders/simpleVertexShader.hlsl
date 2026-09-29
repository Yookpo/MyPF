#include "Common.hlsli"

cbuffer objectConstantBuffer : register(b0)
{
    matrix model;
    matrix invTranspose;
}

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    
    float4 pos = float4(input.pos, 1.0f);
    pos = mul(pos, model);
    
    output.posWorld = pos.xyz; // 월드 위치 따로 저장
    
    pos = mul(pos, view);
    pos = mul(pos, projection);
    
    output.pos = pos;
    output.color = input.color;
    
    float4 normal = float4(input.normal, 0.0f);
    output.normal = mul(normal, invTranspose).xyz;
    output.normal = normalize(output.normal);
    
    output.uv = input.uv;
    
    return output;
}

