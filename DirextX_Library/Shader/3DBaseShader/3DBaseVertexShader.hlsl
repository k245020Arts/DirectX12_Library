#include "../2DBaseShader/BasicShaderHeader.hlsli"

VSOutput BasicVS_3D(VSInput input)
{
    VSOutput output = (VSOutput) 0;

    float4 localPos = float4(input.pos, 1.0f);
    float4 worldPos = mul(World, localPos);
    float4 viewPos = mul(View, worldPos);
    float4 projPos = mul(Proj, viewPos);

    float4 normal = float4(input.normal.xyz, 0.0);
    
    output.svpos = projPos;
    output.color = input.color;
    output.uv = input.uv; // ‚±‚±‚ª•ÏX“_B“ü—Í‚©‚çuv‚ğ“n‚·
    output.normal = mul(World, normal);
    return output;
}