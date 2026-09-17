cbuffer PostProcessConstantData : register(b0)
{
    float exposure;
    float threshold;
    uint toneMapper;
    float pad;
    float dx;
    float dy;
    float bloomStrength;
    float pad2;
}