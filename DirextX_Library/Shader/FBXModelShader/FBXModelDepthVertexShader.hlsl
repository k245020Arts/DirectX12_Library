#include "../2DBaseShader/BasicShaderHeader.hlsli"

#define BONE_MAX 1024

cbuffer BoneBuffer : register(b2)
{
    matrix bones[BONE_MAX];
};

float4 DepthVS_FBX(ModelVSInput input) : SV_POSITION
{
    float4 localPos = float4(input.pos, 1.0f);

    // スキニング
    float4 bonePos =
        mul(bones[input.boneno[0]], localPos) * input.weights[0] +
        mul(bones[input.boneno[1]], localPos) * input.weights[1] +
        mul(bones[input.boneno[2]], localPos) * input.weights[2] +
        mul(bones[input.boneno[3]], localPos) * input.weights[3];

    // ボーンを持たないメッシュへの対応
    float weightSum =
        input.weights[0] +
        input.weights[1] +
        input.weights[2] +
        input.weights[3];

    if (weightSum < 0.001f)
    {
        bonePos = localPos;
    }

    // ワールド変換
    float4 worldPos = mul(World, bonePos);

    // ライト視点のViewProjection変換
    float4 viewPos = mul(View, worldPos);

    float4 projPos = mul(Proj, viewPos);

    return projPos;
}