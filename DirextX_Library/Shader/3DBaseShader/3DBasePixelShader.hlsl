#include "../2DBaseShader/BasicShaderHeader.hlsli"

SamplerState smp : register(s0); // サンプラー
Texture2D _MainTex : register(t0); // テクスチャ

float4 BasicPS_3D(VSOutput input) : SV_TARGET
{
    float4 color = float4(_MainTex.Sample(smp, input.uv)) * input.color;
    
    //画像の透明な部分は描画しない
    clip(color.a - 0.001f);
    return color;
}