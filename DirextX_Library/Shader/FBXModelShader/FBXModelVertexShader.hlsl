#include "../2DBaseShader/BasicShaderHeader.hlsli"

#define BONE_MAX 1024

cbuffer BoneBuffer : register(b2)
{
    matrix bones[BONE_MAX];
};

VSOutput BasicVS_FBX(ModelVSInput input)
{
    VSOutput output = (VSOutput) 0;

    float4 localPos = float4(input.pos, 1.0f);
    float4 bonePos =
    mul(bones[input.boneno[0]], localPos) * input.weights[0] +
    mul(bones[input.boneno[1]], localPos) * input.weights[1] +
    mul(bones[input.boneno[2]], localPos) * input.weights[2] +
    mul(bones[input.boneno[3]], localPos) * input.weights[3];

    //ボーンを持たないメッシュ(ウェイト合計0)は頂点が消えるのでフォールバック
    float wsum = input.weights[0] + input.weights[1] + input.weights[2] + input.weights[3];
    if (wsum < 0.001f)
        bonePos = localPos;

    //法線も同様に変換
    float3 skinNormal =
    mul((float3x3) bones[input.boneno[0]], input.normal) * input.weights[0] +
    mul((float3x3) bones[input.boneno[1]], input.normal) * input.weights[1] +
    mul((float3x3) bones[input.boneno[2]], input.normal) * input.weights[2] +
    mul((float3x3) bones[input.boneno[3]], input.normal) * input.weights[3];
    
    //float4 bonePos = float4(input.pos, 1.0f);
    
    float4 worldPos = mul(World, bonePos);
    float4 viewPos = mul(View, worldPos);
    float4 projPos = mul(Proj, viewPos);

    float4 normal = float4(input.normal.xyz,0.0);
    
    output.svpos = projPos;
    output.color = input.color;
    output.uv = input.uv; // ここが変更点。入力からuvを渡す
    float3 n = normalize(mul(World, skinNormal).xyz);
    output.normal = float4(n, 0.0f);
    
    return output;
}