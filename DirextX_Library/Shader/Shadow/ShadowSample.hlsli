#ifndef SHADOW_SAMPLING_HLSLI
#define SHADOW_SAMPLING_HLSLI

Texture2D<float> shadowMapTexture : register(t1);
SamplerComparisonState shadowComSampler : register(s2);

// PCFの半径(テクセル単位)。1 = 3x3(9回), 2 = 5x5(25回)
#define SHADOW_PCF_RADIUS 1

float CalculateShadow(float4 shadowPosition, float bias)
{
    float3 projCoord = shadowPosition.xyz / shadowPosition.w;
    float2 shadowUV = float2(projCoord.x * 0.5f + 0.5f, -projCoord.y * 0.5f + 0.5f);

    // ShadowMapの範囲内か判定
    float2 uvValid = step(0.0f, shadowUV) * step(shadowUV, 1.0f);
    float depthValid = step(0.0f, projCoord.z) * step(projCoord.z, 1.0f);
    float valid = uvValid.x * uvValid.y * depthValid;

    // 1テクセルのUV幅(CBを増やさずテクスチャから取得)
    uint w, h;
    shadowMapTexture.GetDimensions(w, h);
    float2 texel = 1.0f / float2(w, h);

    float sum = 0.0f;
    //ループの状態を、普通に並べた状態に変換が出来る。
    //ループの条件等の処理が発生しなくなるため処理負荷につながる
    [unroll]
    for (int y = -SHADOW_PCF_RADIUS; y <= SHADOW_PCF_RADIUS; ++y)
    {
        [unroll]
        for (int x = -SHADOW_PCF_RADIUS; x <= SHADOW_PCF_RADIUS; ++x)
        {
            sum += shadowMapTexture.SampleCmpLevelZero(
                shadowComSampler, shadowUV + float2(x, y) * texel, projCoord.z - bias);
        }
    }
    const float count = (SHADOW_PCF_RADIUS * 2 + 1) * (SHADOW_PCF_RADIUS * 2 + 1);

    return lerp(1.0f, sum / count, valid);
}

#endif