#include "../2DBaseShader/BasicShaderHeader.hlsli"
#include "../Light/Light.hlsli"
#include "../Shadow/ShadowSample.hlsli"

SamplerState smp : register(s0); // サンプラー
Texture2D _MainTex : register(t0); // テクスチャ

SamplerState shadowSampler : register(s1);

SamplerState normalSmp : register(s3); // ノーマルマップのサンプラー
Texture2D _normalTex : register(t2); // ノーマルマップのテクスチャ

ScenePSOutput BasicPS_FBX(VSOutput input) : SV_TARGET
{
    ScenePSOutput output;
    output.color = float4(0, 0, 0, 1);
    output.bloom = float4(0, 0, 0, 0);

    if (input.instNo == 1)
    {
        return output;
    }
    
    //Normalの計算
    float4 normalColor = _normalTex.Sample(normalSmp, input.uv);
    
    normalColor.xy = normalColor.xy * 2.0f - 1.0f;
    
    normalColor.xy *= normalStrength;
    
    normalColor.z = sqrt(saturate(1.0 - dot(normalColor.xy, normalColor.xy)));
                
    float3 T = normalize(input.worldTangent.xyz);
    float3 B = normalize(input.worldBinormal.xyz);
    float3 N = normalize(input.normal.xyz);
    float3x3 TBN = float3x3(T, B, N);

    float3 worldNormal = normalize(mul(normalColor.xyz, TBN));

    float3 useNormal = normalize(lerp(N, worldNormal, enableNormal));
    
    //return float4(useNromal, 1.0f);
     
    //Diffuseの計算
    float3 diffuseLight = ambientColor * ambientIntensity;
    float3 diffuseLightNoShadow = 0;
    float3 specularLight = 0;
    float3 specularLightNoShadow = 0;
    float3 vieDir = normalize(cameraPos - input.worldPosition.xyz);
    float shininess = max(specularPow, 1.0f);

    uint count = min(lightCount, MAX_LIGHT_COUNT);

    [loop]
    for (uint i = 0; i < count; ++i)
    {
        // 方向と、減衰込みの光の色はライトの種類に応じて EvaluateLight が返す
        float3 lightDir;
        float3 lightCol = LightCalculation(lights[i], input.worldPosition.xyz, lightDir);

        float ndl = saturate(dot(useNormal, lightDir));

        //対象となる二つのベクトルを足すとその二つのベクトルをハーフベクトルになる
        float3 halfDir = normalize(lightDir + vieDir);
        float spec = pow(saturate(dot(useNormal, halfDir)), shininess);
        spec *= (ndl > 0.0f) ? 1.0f : 0.0f; // ライトが当たらない面にはハイライトを出さない

        if (lights[i].lightType == LIGHT_TYPE_DIRECTIONAL)
        {
            diffuseLight += lightCol * ndl;
            specularLight += lightCol * spec;
        }
        else
        {
            diffuseLightNoShadow += lightCol * ndl;
            specularLightNoShadow += lightCol * spec;
        }
    }

    float4 color = _MainTex.Sample(smp, input.uv) * input.color;

    // 画像の透明な部分は描画しない
    clip(color.a - 0.001f);

    float shadowColor = CalculateShadow(input.shadowPosition, 0.001f); // 今の値のまま
    float shadowFactor = lerp(0.3f, 1.0f, shadowColor);

    // 全てのライトを合成(影はディレクショナルライトと環境光にだけ掛ける)
    float3 finalRGB = color.rgb * (diffuseLight * shadowFactor + diffuseLightNoShadow);

    float3 specularSum = specularLight * shadowColor + specularLightNoShadow;
    finalRGB += specularSum * specularColor.rgb * specularStrength * enableSpecular;
    
    output.color = float4(finalRGB, color.a);

    // 光らせたい色。しきい値(threshold)はブルームのエフェクト側で掛ける
    output.bloom = float4(finalRGB * enableBloom * bloomIntensity, 1.0f);

    return output;

}