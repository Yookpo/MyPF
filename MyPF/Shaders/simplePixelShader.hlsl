#include "Common.hlsli"
#include "Lighting.hlsli"

Texture2D albedoTexture : register(t0);
SamplerState linearSampler : register(s0);

cbuffer MaterialConstantBuffer : register(b2)
{
    float3 baseColor;
    float roughness;
    float3 emissiveColor;
    float emissiveIntensity;
    float3 rimColor;
    float rimIntensity;
    float rimPower;
    float specular;
    float2 pad3;
}

float4 main(PS_INPUT input) : SV_TARGET
{
    IncidentLight dirLight = GetDirectionalLight();
    
    float3 normal = normalize(input.normal);
    float diffuse = saturate(dot(normal, dirLight.direction));
    
    float3 albedo = albedoTexture.Sample(linearSampler, input.uv).rgb;
    float3 surfaceColor = albedo * baseColor;
    
    float3 ambientColor = surfaceColor * ambientStrength;
    float3 diffuseColor = surfaceColor * dirLight.radiance * diffuse;
    float3 emissive = emissiveColor * emissiveIntensity;
    
    float3 viewDir = normalize(cameraPos - input.posWorld);
    float rimFactor = pow(1.0 - saturate(dot(normal, viewDir)), rimPower);
    float3 rim = rimColor * rimIntensity * rimFactor;
    
    float3 pointLightColor = float3(0, 0, 0);
    for (uint i = 0; i < pointLightCount; i++)
    {
        IncidentLight pointLight = GetPointLight(i, input.posWorld);
        pointLightColor += surfaceColor * pointLight.radiance * saturate(dot(normal, pointLight.direction));
    }
    
    float3 finalColor = ambientColor + diffuseColor + pointLightColor + emissive + rim;
    
    return float4(finalColor, 1.0f);
}

