struct VS_INPUT
{
    float3 position : POSITION0;
    float3 color : COLOR0;
    float3 normal : NORMAL0;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 color : COLOR0;
    float3 normal : NORMAL0;
};


float4 main(PS_INPUT input) : SV_TARGET
{
    input.normal = normalize(input.normal);
    input.normal *= 0.5;
    input.normal += 0.5;
    
    return float4(input.normal, 1.0f);
}

