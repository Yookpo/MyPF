#define NUM_POINT_LIGHTS 8

struct PointLight
{
    float3 position;
    float range;
    float3 color;
    float intensity;
    uint isEnabled;
    float3 pad;
};

float CalcAttenuation(float lightDistance, float range)
{
    // Linear falloff
    if (range <= 0)
    {
        return 0;
    }
    
    lightDistance /= range;
    
    float aten = saturate(lightDistance);
    
    aten = (1 - aten);
    
    return (aten * aten);
}

float3 ComputePointLight(PointLight pointLight, float3 pos, float3 normal, float3 surfaceColor)
{
    if (pointLight.isEnabled == 0)
    {
        return float3(0, 0, 0);
    }
    
    float3 lightVec = pointLight.position - pos;
    // 쉐이딩할 지점부터 조명까지의 거리 
    float d = length(lightVec);
    
    // 너무 멀면 조명 적용 X
    if (d >= pointLight.range || d <= 0.0001f)
    {
        return float3(0, 0, 0);
    }

    // 거리가 0에 가까운 경우를 대비해 조명 방향 정규화
    lightVec = normalize(lightVec);
    float diffuse = saturate(dot(normal, lightVec));
    float aten = CalcAttenuation(d, pointLight.range);

    return surfaceColor * pointLight.color * pointLight.intensity * diffuse * aten;
    
}