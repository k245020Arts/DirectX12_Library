#include "../2DBaseShader/BasicShaderHeader.hlsli"

VSOutput TexVsMain(VSInput input)
{
    VSOutput output = (VSOutput) 0; // アウトプット構造体を定義する

    float4 localPos = float4(input.pos, 1.0f); // 頂点座標
    float4 worldPos = mul(World, localPos); // ワールド座標に変換
    float4 viewPos = mul(View, worldPos); // ビュー座標に変換
    float4 projPos = mul(Proj, viewPos); // 投影変換

    output.svpos = projPos; // 投影変換された座標をピクセルシェーダーに渡す
    output.color = input.color; // 頂点色をそのままピクセルシェーダーに渡す
    
   // UVの値を書き換え
    // input.uv の 0.0～1.0 を、uvRectで指定した範囲に変換する(0.4～0.7の場合、0.4を0.0にして0.7を1.0にするようにする)
    //画像のUVを四角形ポリゴンのUVに対応させるため(画像のUV座標0.4を左上に出したい、だから四角形の左上である0.0に値を変える必要がある)
    output.uv.x = uvRect.x + input.uv.x * uvRect.z;
    output.uv.y = uvRect.y + input.uv.y * uvRect.w;

    
    return output;
}