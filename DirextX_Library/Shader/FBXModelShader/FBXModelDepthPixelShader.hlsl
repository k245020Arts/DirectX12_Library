Texture2D<float> depthTex : register(t0);
SamplerState smp : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 ShadowMapDebugPS(PSInput input) : SV_TARGET
{
    float depth = depthTex.Sample(smp, input.uv);

    return float4(depth, depth, depth, 1.0f);
}