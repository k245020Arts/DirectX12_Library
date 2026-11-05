Texture2D<float4> sceneTexture : register(t0);
Texture2D<float4> normalTexture : register(t1);

SamplerState smp : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 DistortionPixelShader(PSInput input) : SV_TARGET
{
    float2 uv = input.uv;

    float3 normal = normalTexture.Sample(smp, uv).xyz * 2.0f - 1.0f;

    float2 distortion = normal.xy * 0.1f;

    float2 distortedUV = uv + distortion;

    return sceneTexture.Sample(smp, distortedUV);
}