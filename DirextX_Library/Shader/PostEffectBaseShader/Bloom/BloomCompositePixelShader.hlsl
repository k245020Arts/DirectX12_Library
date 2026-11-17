Texture2D SceneTex : register(t0); // 入力画像(前段の結果)
Texture2D BloomTex : register(t1); // ぼかした光
SamplerState MainSampler : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

// C++ の BloomCompositeParam と同じ並び
cbuffer BloomCompositeParam : register(b0)
{
    float intensity;
    float3 padding;
};

float4 BloomCompositePS(PSInput input) : SV_TARGET
{
    float4 scene = SceneTex.Sample(MainSampler, input.uv);
    float3 bloom = BloomTex.Sample(MainSampler, input.uv).rgb;

    // 光を足す(1.0 を超えた分は出力フォーマットで切り捨てられる)
    return float4(scene.rgb + bloom * intensity, scene.a);
}
