#include "../2DBaseShader/BasicShaderHeader.hlsli"

SamplerState smp : register(s0); // サンプラー
Texture2D _MainTex : register(t0); // テクスチャ

float4 BasicPS_FBX(VSOutput input) : SV_TARGET
{
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

    return float4(finalRGB, color.a);
}