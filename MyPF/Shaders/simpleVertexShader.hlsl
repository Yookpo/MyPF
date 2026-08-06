cbuffer constantBuffer : register(b0)
{
    matrix model;
}


struct VS_INPUT
{
    float3 position : POSITION0;
    float3 color : COLOR0;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 color : COLOR0;
};



PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    
    output.position = float4(input.position, 1.0f);
    output.position = mul(output.position, model);
    
    output.color = input.color;
    
    return output;
}