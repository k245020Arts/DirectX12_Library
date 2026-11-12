#include "../2DBaseShader/BasicShaderHeader.hlsli"
#include "../Shadow/ShadowSample.hlsli"

SamplerState smp : register(s0); // サンプラー
Texture2D _MainTex : register(t0); // テクスチャ

SamplerState shadowSampler : register(s1);

SamplerState normalSmp : register(s3); // ノーマルマップのサンプラー
Texture2D _normalTex : register(t2); // ノーマルマップのテクスチャ

float4 BasicPS_FBX(VSOutput input) : SV_TARGET
{
    static const float SPECULAR_STRENGTH = 0.3f; // スペキュラの強さ
    static const float3 SPECULAR_COLOR = float3(1.0f, 1.0f, 1.0f); // ハイライトの色
    
    if (input.instNo == 1)
    {
        return float4(0, 0, 0, 1);
    }
    
    //Normalの計算
    float4 normalColor = _normalTex.Sample(normalSmp, input.uv);
    
    normalColor.xy = normalColor.xy * 2.0f - 1.0f;
    
    float normalStrength = 1.0f;
    normalColor.xy *= normalStrength;
    
    normalColor.z = sqrt(saturate(1.0 - dot(normalColor.xy, normalColor.xy)));
                
    float3 T = normalize(input.worldTangent.xyz);
    float3 B = normalize(input.worldBinormal.xyz);
    float3 N = normalize(input.normal.xyz);
    float3x3 TBN = float3x3(T, B, N);

    float3 worldNormal = normalize(mul(normalColor.xyz, TBN));

    float3 useNormal = worldNormal;
    
    //return float4(useNromal, 1.0f);
     
    //Diffuseの計算
    float3 diffuseLight = ambientColor * ambientIntensity;
    float3 specularLight = 0;
    float3 vieDir = normalize(cameraPos - input.worldPosition.xyz);
    
    [unroll(MAX_LIGHT_COUNT)]
    for (uint i = 0; i < lightCount; ++i)
    {
        float3 lightDir = normalize(-lights[i].direction);

        float3 lightCol = lights[i].color * lights[i].intensity;

        float ndl = saturate(dot(useNormal, lightDir));

         //対象となる二つのベクトルを足すとその二つのベクトルをハーフベクトルになる
        float3 halfDir = normalize(lightDir + vieDir);
        float spec = pow(saturate(dot(useNormal, halfDir)), 1.0f);
        //spec *= (ndl > 0.0f) ? 1.0f : 0.0f; // ライトが当たらない面にはハイライトを出さない

        diffuseLight += lightCol * ndl;
        specularLight += lightCol * spec;
    }

    float4 color = _MainTex.Sample(smp, input.uv) * input.color;

    // 画像の透明な部分は描画しない
    clip(color.a - 0.001f);

    // 両方のライトを合成
    float3 finalRGB = color.rgb * diffuseLight;

    float shadowColor = CalculateShadow(input.shadowPosition, 0.001f); // 今の値のまま
    finalRGB.rgb *= lerp(0.3f, 1.0f, shadowColor);
    
    finalRGB += specularLight * SPECULAR_COLOR * SPECULAR_STRENGTH * shadowColor;
    
    return float4(finalRGB, color.a);

}