#include "../2DBaseShader/BasicShaderHeader.hlsli"

Texture2D<float4> tex : register(t0); //0番スロットに設定されたテクスチャ
SamplerState smp : register(s0); //0番スロットに設定されたサンプラ

float4 TexPsMain(VSOutput input) : SV_TARGET
{
    float4 color = float4(tex.Sample(smp, input.uv)) * input.color;
    
    //画像の透明な部分は描画しない
    clip(color.a - 0.001f);
    return color;
}