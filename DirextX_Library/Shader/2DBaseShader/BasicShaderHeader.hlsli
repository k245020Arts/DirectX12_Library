#ifndef BASIC_SHADER_HEADER
#define BASIC_SHADER_HEADER

cbuffer Transform : register(b0)
{
    float4x4 World; // ワールド行列
    float4x4 View; // ビュー行列
    float4x4 Proj; // 投影行列
    float4x4 shadow; //影行列
    Matrix lightViewProjection;
    
    float4 uvRect;
}

// ノーマルマップ用用(C++ の NormalMapParam と同じ並び)
cbuffer NormalMapParam : register(b3)
{
    float enableNormal;
    float normalStrength;
    float2 normalPad;
};

// スペキュラ用(C++ の SpecularParam と同じ並び)
cbuffer SpecularParam : register(b4)
{
    float enableSpecular;
    float specularStrength;
    float specularPow;
    float specularPad;
    
    float4 specularColor;
};

// ブルーム用(C++ の BloomParam と同じ並び)
cbuffer BloomParam : register(b5)
{
    float enableBloom;
    float bloomIntensity;
    float2 bloomPad;
};

cbuffer HeightMapParam : register(b6)
{
    float enableHeight;
    float heightStrength;
    float2 heightPad;
};

// シーン描画(マルチレンダーターゲット)のピクセルシェーダー出力
// 並びは C++ の SceneLayout(SceneLayout.h)と同じにすること
struct ScenePSOutput
{
    float4 color : SV_TARGET0; // 通常の描画結果
    float4 bloom : SV_TARGET1; // 光らせたい色(何も光らないなら 0)
};

struct VSInput
{
    float3 pos : POSITION; // 頂点座標
    float4 normal : NORMAL; // 法線
    float2 uv : TEXCOORD; // UV
    float4 tangent : TANGENT; // 接空間
    float4 color : COLOR; // 頂点色
};

struct ModelVSInput
{
    float3 pos : POSITION; // 頂点座標
    float3 normal : NORMAL; // 法線
    float2 uv : TEXCOORD; // UV
    float4 tangent : TANGENT; // 接空間
    float4 color : COLOR; // 頂点色
    uint4 boneno : BONENO; //ボーン番号
    float4 weights : WEIGHT; //ボーン番号
};

struct VSOutput
{
    float4 svpos : SV_POSITION; // 変換された座標
    float4 color : COLOR; // 変換された色
    float2 uv : TEXCOORD; //uv座標
    float3 worldTangent : TANGENT; //接空間
    float3 worldBinormal : BINORMAL; //
    float4 normal : NORMAL; // 法線
    uint instNo : SV_InstanceID;//インスタンシング番号
    float4 shadowPosition : TEXCOORD1;
    float4 worldPosition : TEXCOORD2;
};

#endif