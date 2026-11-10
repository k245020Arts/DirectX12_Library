struct DepthVSInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 tangent : TANGENT;
    float4 color : COLOR;
    uint4 boneno : BONENO;
    float4 weights : WEIGHT;
};

cbuffer Transform : register(b0)
{
    float4x4 World;
    float4x4 View;
    float4x4 Proj;
    float4 uvRect;
};

#define BONE_MAX 1024
cbuffer BoneBuffer : register(b2)
{
    matrix bones[BONE_MAX];
};

float4 DepthVS_FBX(DepthVSInput input) : SV_POSITION
{
    float4 localPos = float4(input.pos, 1.0f);

    float4 bonePos =
        mul(bones[input.boneno[0]], localPos) * input.weights[0] +
        mul(bones[input.boneno[1]], localPos) * input.weights[1] +
        mul(bones[input.boneno[2]], localPos) * input.weights[2] +
        mul(bones[input.boneno[3]], localPos) * input.weights[3];

    float weightSum = input.weights[0] + input.weights[1] +
        input.weights[2] + input.weights[3];
    if (weightSum < 0.001f)
    {
        bonePos = localPos;
    }

    float4 worldPos = mul(World, bonePos);
    float4 lightViewPos = mul(View, worldPos);
    return mul(Proj, lightViewPos);
}
