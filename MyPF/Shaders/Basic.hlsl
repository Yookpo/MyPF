
struct VS_INPUT
{
    float3 position : POSITION0;
    float4 color : COLOR0;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
};


PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT output;
    
    output.position = float4(input.position, 1.0f);
    output.color = input.color;
    
    return output;
}

float4 PSMain(PS_INPUT input)   : SV_TARGET
{
    return input.color;
}