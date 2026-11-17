Texture2D MainTex : register(t0);
SamplerState MainSampler : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

// C++ の BloomBlurParam と同じ並び
cbuffer BloomBlurParam : register(b0)
{
    float4 weights[2]; // ガウス重み(中心 + 片側7個)
    float2 direction; // (1,0)=横 (0,1)=縦
    float offsetScale; // サンプル間隔(テクセル倍率)
    float threshold; // 0以上 : この明るさ以下を捨てる / マイナス : 素通し
};

// 明るさ(輝度)が threshold を超えた分だけ残す
float3 Prefilter(float3 c)
{
    if (threshold < 0.0f)
    {
        return c;
    }

    float lum = dot(c, float3(0.299f, 0.587f, 0.114f));
    float k = saturate((lum - threshold) / max(1.0f - threshold, 0.0001f));

    return c * k;
}

float4 BloomBlurPS(PSInput input) : SV_TARGET
{
    float w, h, levels;
    MainTex.GetDimensions(0, w, h, levels);

    float2 texelSize = float2(1.0f / w, 1.0f / h);

    float3 sum = weights[0].x * Prefilter(MainTex.Sample(MainSampler, input.uv).rgb);

    [unroll]
    for (int i = 1; i < 8; ++i)
    {
        float2 offset = texelSize * direction * i * offsetScale;
        float weight = weights[i >> 2][i % 4];

        sum += weight * Prefilter(MainTex.Sample(MainSampler, input.uv + offset).rgb);
        sum += weight * Prefilter(MainTex.Sample(MainSampler, input.uv - offset).rgb);
    }

    return float4(sum, 1.0f);
}
