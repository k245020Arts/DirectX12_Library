#include "../2DBaseShader/BasicShaderHeader.hlsli"
#include "../Light/Light.hlsli"
#include "../Shadow/ShadowSample.hlsli"

SamplerState smp : register(s0); // サンプラー
Texture2D _MainTex : register(t0); // テクスチャ

SamplerState shadowSampler : register(s1);

ScenePSOutput BasicPS_3D(VSOutput input) : SV_TARGET
{
    ScenePSOutput output;
    output.color = float4(0, 0, 0, 1);

    // このシェーダーのものは光らない。0 を書いて、後ろにある光るものを隠す
    output.bloom = float4(0, 0, 0, 0);
    
    if (input.instNo == 1)
    {
        return output;
    }
    
    float3 totalLight = ambientColor * ambientIntensity;
    float3 totalLightNoShadow = 0;
    
    [loop]
    for (uint i = 0; i < lightCount; ++i)
    {
        float3 lightDir;
        float3 lightCol = LightCalculation(lights[i], input.worldPosition.xyz, lightDir);

        float brightness = saturate(dot(normalize(input.normal.xyz), lightDir));

        if (lights[i].lightType == LIGHT_TYPE_DIRECTIONAL)
        {
            totalLight += lightCol * brightness;
        }
        else
        {
            totalLightNoShadow += lightCol * brightness;
        }
    }

    float4 color = _MainTex.Sample(smp, input.uv) * input.color;

    // 画像の透明な部分は描画しない
    clip(color.a - 0.001f);

    float shadowColor = CalculateShadow(input.shadowPosition, 0.001f); // 今の値のまま

    // 全てのライトを合成(影はディレクショナルライトと環境光にだけ掛ける)
    float3 finalRGB = color.rgb * (totalLight * lerp(0.3f, 1.0f, shadowColor) + totalLightNoShadow);

    output.color = float4(finalRGB, color.a);

    return output;
}