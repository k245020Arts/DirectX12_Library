struct Output
{
    float4 pos : SV_Position;
    float4 normal : NORMAL; //システム用頂点座標
    float2 uv : TEXCOORD; //uv値
    min16int2 boneno : BONE_NO;
    min16uint weight : WEIGHT;
};

Texture2D<float4> tex : register(t0); //0番スロットに設定されたTexture
SamplerState smp : register(s0); //０番スロットに設定されたサンプラー

cbuffer cbuff0 : register(b0) //定数バッファー
{
    matrix world;
    matrix viewproj;
}