Texture2D MainTex : register(t0);
SamplerState MainSampler : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 CopyPS(PSInput input) : SV_TARGET
{
    return MainTex.Sample(MainSampler, input.uv);
}
