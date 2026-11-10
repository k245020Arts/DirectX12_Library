Texture2D<float> depthTex : register(t0);
SamplerState smp : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 ShadowMapDebugPS(PSInput input) : SV_TARGET
{
    // 直交投影の深度値（0～1）をそのままグレースケール表示する。
    float depth = depthTex.SampleLevel(smp, input.uv, 0);
    return float4(depth, depth, depth, 1.0f);
}
