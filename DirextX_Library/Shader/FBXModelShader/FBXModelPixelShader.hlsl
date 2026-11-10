#include "../2DBaseShader/BasicShaderHeader.hlsli"

SamplerState smp : register(s0); // サンプラー
Texture2D _MainTex : register(t0); // テクスチャ

Texture2D<float> shadowMapTexture : register(t1);
SamplerState shadowSampler : register(s1);
SamplerComparisonState shadowComSampler : register(s2);

float CalculateShadow(float4 shadowPosition)
{
    float3 projCoord = shadowPosition.xyz / shadowPosition.w;

    float2 shadowUV = float2( projCoord.x * 0.5f + 0.5f, -projCoord.y * 0.5f + 0.5f);

    float bias = 0.001f;

    // ShadowMapの範囲内か判定
    float2 uvValid = step(0.0f, shadowUV) * step(shadowUV, 1.0f);

    float depthValid = step(0.0f, projCoord.z) * step(projCoord.z, 1.0f);

    float valid = uvValid.x * uvValid.y * depthValid;

    // 比較サンプラーによる深度比較
    float shadow = shadowMapTexture.SampleCmpLevelZero(shadowComSampler,shadowUV,projCoord.z - bias);

    return lerp(1.0f, shadow, valid);
}

float4 BasicPS_FBX(VSOutput input) : SV_TARGET
{
    if (input.instNo == 1)
    {
        return float4(0, 0, 0, 1);
    }
    float3 totalLight = ambientColor * ambientIntensity;

    for (uint i = 0; i < lightCount; ++i)
    {
        float3 lightDir = normalize(-lights[i].direction);

        float brightness = saturate(dot(normalize(input.normal.xyz), lightDir));

        totalLight += lights[i].color * lights[i].intensity * brightness;
    }

    float4 color = _MainTex.Sample(smp, input.uv) * input.color;

    // 画像の透明な部分は描画しない
    clip(color.a - 0.001f);

    // 両方のライトを合成
    float3 finalRGB = color.rgb * totalLight;
    
    float shadow = CalculateShadow(input.shadowPosition);

    finalRGB.rgb *= lerp(0.3f, 1.0f, shadow);

    return float4(finalRGB, color.a);
    
    return float4(shadow, shadow, shadow, 1.0f);

}