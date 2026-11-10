#include "../2DBaseShader/BasicShaderHeader.hlsli"

VSOutput BasicVS_3D(VSInput input, uint instNo : SV_InstanceID)
{
    VSOutput output = (VSOutput) 0;

    float4 localPos = float4(input.pos, 1.0f);
    float4 worldPos = mul(World, localPos);
    float4 shadowPos = worldPos;
    if (instNo == 1)
    {
        shadowPos = mul(shadow, worldPos);
    }
    float4 viewPos = mul(View, shadowPos); //影を出す設定をしてなかったらワールド座標
    float4 projPos = mul(Proj, viewPos);

    float4 normal = float4(input.normal.xyz, 0.0);
    
    output.svpos = projPos;
    output.color = input.color;
    output.uv = input.uv; // ここが変更点。入力からuvを渡す
    output.normal = mul(World, normal);
    output.instNo = instNo;
    output.shadowPosition = mul(lightViewProjection, worldPos);
    
    return output;
}