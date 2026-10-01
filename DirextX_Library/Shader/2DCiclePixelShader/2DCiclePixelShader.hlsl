#include "../2DBaseShader/BasicShaderHeader.hlsli"

cbuffer Transform : register(b0)
{
    float wireFrame; //ワイヤーフレーム用の変数
}

float4 Cicle2DPixelShader(VSOutput input) : SV_TARGET
{
	 //1中心からの距離を計算
    float dist = length(input.uv - float2(0.5, 0.5));

    //2円の外側をカット
    clip(0.5 - dist);

    //3外枠モードのときだけ「内側」をくり抜く
    clip((dist - 0.48) * wireFrame);

    return input.color;
}