#ifndef LIGHT_SHADER_HEADER
#define LIGHT_SHADER_HEADER

#define MAX_LIGHT_COUNT 12

//ライトの種類
#define LIGHT_TYPE_DIRECTIONAL 1
#define LIGHT_TYPE_POINT       2
#define LIGHT_TYPE_SPOT        3

struct LightData
{
    float3 direction; // 光の進む向き (Directional / Spot)
    float intensity;

    float3 color;
    uint lightType;

    float3 position; // ワールド座標 (Point / Spot)
    float range; // 光の届く距離 (Point / Spot)

    float innerCos; // スポットの内側角度の cos
    float outerCos; // スポットの外側角度の cos
    float2 lightPad;
};

cbuffer LightBuffer : register(b1)
{
    LightData lights[MAX_LIGHT_COUNT];

    uint lightCount;
    float3 padding;

    float3 ambientColor;
    float ambientIntensity;
    
    float3 cameraPos;
    float pad;
};

float DistanceAttention(float dist,float range)
{
    float r = max(range, 0.00001f);
    float t = saturate(dist / r);
    float window = saturate(1.0f - t * t * t * t);
    return window * window;
}

float3 LightCalculation(LightData light, float3 worldPos, out float3 outL)
{
    outL = float3(0.0f, 1.0f, 0.0f); // 警告よけの初期値
    
    float3 radiance = light.color * light.intensity;
    
    // ディレクショナル: 距離減衰なし
    if (light.lightType == LIGHT_TYPE_DIRECTIONAL)
    {
        outL = normalize(-light.direction);
        return radiance;
    }
    
    float3 toLight = light.position - worldPos;
    float dist = length(toLight);
    outL = toLight / max(dist, 0.00001f);
    
    float attention = DistanceAttention(dist, light.range);
    
    if(light.lightType == LIGHT_TYPE_SPOT)
    {
        float cosAngle = dot(-outL, normalize(light.direction));
        attention *= smoothstep(light.outerCos, light.innerCos, cosAngle);
    }
    return radiance * attention;
}

#endif