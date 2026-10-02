#include "../2DBaseShader/BasicShaderHeader.hlsli"

float4 Cicle2DPixelShader(VSOutput input) : SV_TARGET
{
	 //1中心からの距離を計算
    float dist = length(input.uv - float2(0.5, 0.5));

    //2円の外側をカット
    clip(0.5 - dist);

    return input.color;
}