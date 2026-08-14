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


float4 main(PS_INPUT input) : SV_TARGET
{
    return float4(input.color, 1.0f);
}

