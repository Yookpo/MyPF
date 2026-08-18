cbuffer objectConstantBuffer : register(b0)
{
    matrix model;
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
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
    float3 normal : NORMAL0;
};



PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    
    float4 pos = float4(input.pos, 1.0f);
    pos = mul(pos, model);
    pos = mul(pos, view);
    pos = mul(pos, projection);
    
    output.pos = pos;
    output.color = input.color;
    output.normal = input.normal;
    
    return output;
}

