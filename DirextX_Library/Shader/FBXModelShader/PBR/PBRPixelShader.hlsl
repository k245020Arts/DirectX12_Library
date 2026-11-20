#include "../../Shadow/ShadowSample.hlsli"
#include "../../2DBaseShader/BasicShaderHeader.hlsli"

Texture2D<float4> albedoTexture : register(t0);
Texture2D<float4> normalTexture : register(t2);

Texture2D<float> metallicTexture : register(t4);
Texture2D<float> roughnessTexture : register(t5);
Texture2D<float> aoTexture : register(t6);

SamplerState materialSampler : register(s0);

static const float PI = 3.14159265f;

cbuffer PBRMaterialParam : register(b7)
{
    float3 materialAlbedo;
    float materialMetallic;

    float materialRoughness;
    float materialAO;
    uint useAlbedoMap;
    uint useMetallicMap;

    uint useRoughnessMap;
    uint useAOMap;
    float2 pbrPadding;
};

float DistributionGGX(float3 N,float3 H,float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;

    float NdotH = saturate(dot(N, H));

    float NdotH2 = NdotH * NdotH;

    float denominator = NdotH2 * (a2 - 1.0f) + 1.0f;

    return a2 / max(3.14159265f * denominator * denominator,0.000001f);
}

float GeometrySchlickGGX(float NdotV,float roughness)
{
    float r = roughness + 1.0f;
    float k = (r * r) / 8.0f;

    return NdotV /max(NdotV * (1.0f - k) + k,0.000001f);
}

float GeometrySmith(float3 N,float3 V,float3 L,float roughness)
{
    float NdotV = saturate(dot(N, V));
    float NdotL = saturate(dot(N, L));

    float ggxV = GeometrySchlickGGX(NdotV, roughness);

    float ggxL = GeometrySchlickGGX(NdotL, roughness);

    return ggxV * ggxL;
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    //一般的なSchlick近似式 : \[F = F_0 + (1 - F_0)(1 - \cos\theta) ^ 5\]
    return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
}

float3 CalculatePBRLighting(float3 N,float3 V,float3 L,float3 albedo,float metallic,float roughness,float3 lightColor,float lightIntensity)
{
    float NdotL = saturate(dot(N, L));
    float NdotV = saturate(dot(N, V));

    if (NdotL <= 0.0f)
    {
        return float3(0.0f, 0.0f, 0.0f);
    }

    float3 H = normalize(V + L);

    //フレネル反射、金属と非金属を決める部分
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f),albedo,metallic);

    //見る角度によって反射率を変えるために書く
    float3 F = FresnelSchlick(saturate(dot(H, V)),F0);

    //鏡面反射の設定 : ラフネスのざらざら度を使い反射の方向を制御している
    float D = DistributionGGX(N, H, roughness);

    //自己遮蔽の計算、凹凸によって光が遮られたりする現象
    float G = GeometrySmith(N, V, L, roughness);

    //Cook-Torranceの鏡面反射の計算式（スペキュラ反射に相当するもの）
    float3 specular =(D * G * F) /max(4.0f * NdotV * NdotL, 0.001f);

    //拡散反射の計算
    //拡散反射に回す光の割合を計算している
    float3 kD = (1.0f - F) * (1.0f - metallic);

    //Albedoは物体本来の色。
    //例えば赤い物体なら、 拡散反射した光にも赤色が反映される。
    // /PIはランバート反射の正規化に必要になるので絶対に入れる
    float3 diffuse = kD * albedo / PI;

    float3 radiance = lightColor * lightIntensity;

    return (diffuse + specular) * radiance * NdotL;
}

ScenePSOutput main(VSOutput _output, uint instNo : SV_InstanceID)
{
    ScenePSOutput output;

    float3 albedoTex = albedoTexture.Sample(materialSampler, _output.uv).rgb;

    //Leapで必要か不必要の判定
    float3 vertexColor = _output.color.rgb;

    float3 albedo = lerp(materialAlbedo,materialAlbedo * albedoTex,(float) useAlbedoMap);

    albedo *= vertexColor;

    float metallicTex = metallicTexture.Sample(materialSampler, _output.uv).r;

    //Leapで必要か不必要の判定
    float metallic = lerp(materialMetallic,materialMetallic * metallicTex,(float) useMetallicMap);

    
    float roughnessTex = roughnessTexture.Sample(materialSampler, _output.uv).r;

    //Leapで必要か不必要の判定
    float roughness = lerp(materialRoughness,materialRoughness * roughnessTex,(float) useRoughnessMap);

    roughness = clamp(roughness, 0.04f, 1.0f);
    
    float aoTex = aoTexture.Sample(materialSampler, _output.uv).r;
    
     //Leapで必要か不必要の判定
    float ao = lerp(materialAO,materialAO * aoTex,(float) useAOMap);

    // --------------------------------
    // 2. NormalMap
    // --------------------------------

    float3 baseNormal = normalize(_output.normal);

    float3 normalSample =
    normalTexture.Sample(materialSampler, _output.uv).xyz;

    normalSample = normalSample * 2.0f - 1.0f;

    // ノーマルマップの強さ
    normalSample.xy *= normalStrength;

    normalSample = normalize(normalSample);

    float3x3 TBN = float3x3(normalize(_output.worldTangent),normalize(_output.worldBinormal),baseNormal);

    float3 mappedNormal = normalize(mul(normalSample, TBN));

    // 有効フラグと強さを反映
    float3 N = normalize(lerp(baseNormal, mappedNormal, saturate(enableNormal)));
    
    float3 V = normalize(cameraPos - _output.worldPosition.xyz);

    // --------------------------------
    // 4. ShadowMap
    // --------------------------------

    float shadowColor = CalculateShadow(_output.shadowPosition, 0.001f); // 今の値のまま

    float3 directLighting = float3(0, 0, 0);

    // C++側のライト最大数を超えないようにする
    uint count = lightCount;

    for (uint i = 0; i < count; ++i)
    {
        float3 L = normalize(-lights[i].direction);

        float3 lightResult = CalculatePBRLighting(N,V,L,albedo,metallic,roughness,lights[i].color,lights[i].intensity);

        directLighting += lightResult;
    }
    
    directLighting *= lerp(0.3f, 1.0f, shadowColor);
    
    // --------------------------------
    // 6. 環境光
    // --------------------------------

    float3 ambient = albedo * ambientColor * ambientIntensity * ao;

    // --------------------------------
    // 7. 最終カラー
    // --------------------------------

    float3 finalColor = directLighting + ambient;

    output.color = float4(finalColor, 1.0f);

    // --------------------------------
    // 8. Bloom
    // --------------------------------

    // 光らせたい色。しきい値(threshold)はブルームのエフェクト側で掛ける
    output.bloom = float4(finalColor * enableBloom * bloomIntensity, 1.0f);
    
    return output;
}