#include "../2DBaseShader/BasicShaderHeader.hlsli"

SamplerState smp : register(s0); // サンプラー
Texture2D _MainTex : register(t0); // テクスチャ

float4 BasicPS_3D(VSOutput input) : SV_TARGET
{
    return _MainTex.Sample(smp, input.uv) * input.color;
}