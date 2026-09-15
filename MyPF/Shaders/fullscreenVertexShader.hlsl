struct Output
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

// 화면(-1~1)보다 큰 삼각형 하나로 화면 전체를 덮는다
static const float2 edge[3] = { float2(-1, -1), float2(-1, 3), float2(3, -1) };

Output main(uint vertexID : SV_VertexID)
{
    Output output;

    float2 clipXY = edge[vertexID];

    output.position = float4(clipXY, 0.0f, 1.0f);

    // 클립 공간은 y가 위로, UV는 v가 아래로 커지므로 y 부호를 뒤집는다
    output.uv = float2(clipXY.x * 0.5f + 0.5f, -clipXY.y * 0.5f + 0.5f);

    return output;
}
