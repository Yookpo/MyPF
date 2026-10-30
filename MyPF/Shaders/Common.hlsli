#ifndef COMMON_HLSLI
#define COMMON_HLSLI

static const float PI = 3.14159265f;

struct VS_INPUT
{
    float3 pos : POSITION0;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
    float3 tangent : TANGENT0;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 posWorld : TEXCOORD0; // 조명 계산용
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD1;
    float3 tangentWorld : TANGENT0;
};

cbuffer CameraConstantBuffer : register(b1)
{
    matrix view;
    matrix projection;
    float3 cameraPos;
    float cameraPad;
}
#endif

