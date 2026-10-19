#include "../2DBaseShader/BasicShaderHeader.hlsli"

SamplerState smp : register(s0); // サンプラー
Texture2D _MainTex : register(t0); // テクスチャ

float4 BasicPS_3D(VSOutput input) : SV_TARGET
{
    
    float light = normalize(float3(1, -1, 1));
    
    float brightness = dot(-light, input.normal);
    
    float4 color = float4(_MainTex.Sample(smp, input.uv)) * input.color;
    
    //画像の透明な部分は描画しない
    clip(color.a - 0.001f);
    
    float4 finalColor = float4(color.r * brightness, color.g * brightness, color.b * brightness, color.a);
    return finalColor;
}