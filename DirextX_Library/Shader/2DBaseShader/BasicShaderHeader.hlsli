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

#define MAX_LIGHT_COUNT 12

struct LightData
{
    float3 direction;
    float intensity;

    float3 color;
    float padding;
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

cbuffer NormalMapParam : register(b3)
{
    float enableNormal;
    float normalStrength;
    float2 normalPad;
};

cbuffer SpecularParam : register(b4)
{
    float enableSpecular;
    float specularStrength;
    float specularPow;
    float specularPad;
    
    float4 specularColor;
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