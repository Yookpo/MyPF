cbuffer objectConstantBuffer : register(b0)
{
    matrix model;
    matrix invTranspose;
}

cbuffer cameraConstantBuffer : register(b1)
{
    matrix view;
    matrix projection;
}

struct VS_INPUT
{
    float3 pos : POSITION0;
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

