#include "../2DBaseShader/BasicShaderHeader.hlsli"

#define BONE_MAX 256

cbuffer BoneBuffer : register(b2)
{
    matrix bones[BONE_MAX];
};

VSOutput BasicVS_FBX(ModelVSInput input)
{
    VSOutput output = (VSOutput) 0;

    float4 localPos = float4(input.pos, 1.0f);
    float4 bonePos = mul(bones[input.boneno[0]], localPos);
    float4 worldPos = mul(World, bonePos);
    float4 viewPos = mul(View, worldPos);
    float4 projPos = mul(Proj, viewPos);

    input.normal.w = 0.0;
    
    output.svpos = projPos;
    output.color = input.color;
    output.uv = input.uv; // ‚±‚±‚ª•ÏX“_B“ü—Í‚©‚çuv‚ğ“n‚·
    output.normal = mul(World, input.normal);
    return output;
}