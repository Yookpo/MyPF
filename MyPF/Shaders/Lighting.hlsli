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
