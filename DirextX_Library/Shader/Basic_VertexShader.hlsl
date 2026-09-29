#include "BasicShaderHeader.hlsli"

VSOutput BasicVS(VSInput input)
{
    VSOutput output = (VSOutput) 0; // アウトプット構造体を定義する

    float4 localPos = float4(input.pos, 1.0f); // 頂点座標
    float4 worldPos = mul(World, localPos); // ワールド座標に変換
    float4 viewPos = mul(View, worldPos); // ビュー座標に変換
    float4 projPos = mul(Proj, viewPos); // 投影変換

    output.svpos = projPos; // 投影変換された座標をピクセルシェーダーに渡す
    output.color = input.color; // 頂点色をそのままピクセルシェーダーに渡す
    
    return output;
    
}
