#include "../2DBaseShader/BasicShaderHeader.hlsli"

float4 Cicle2DPixelShader(VSOutput input) : SV_TARGET
{
	//中心からの距離を計算
    float distance = length(input.uv - float2(0.5, 0.5));

    //距離が0.5を超えたらピクセルを破棄
    clip(0.5 - distance);

    return input.color;
}