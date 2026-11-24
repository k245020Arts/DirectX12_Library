cbuffer MotionBlurParam : register(b0)
{
    float4x4 gInvCurrViewProj; // 現フレームのViewProj逆行列
    float4x4 gPrevViewProj; // 前フレームのViewProj行列
    float gBlurStrength; // ブラーの強さ（例: 0.5 ~ 1.0）
    int gSampleCount; // サンプル数（例: 8 ~ 16）
    float2 motionPadding;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};


Texture2D<float4> ColorTex : register(t0); // シーンのカラー画像
Texture2D<float> DepthTex : register(t1); // 深度バッファ（R32_FLOATなど）
SamplerState LinearClamp : register(s0);

float4 GetCurrentClipPos(float2 uv, float depth)
{
    float4 ndc;
    ndc.x = uv.x * 2.0f - 1.0f;
    ndc.y = (1.0f - uv.y) * 2.0f - 1.0f;
    ndc.z = depth;
    ndc.w = 1.0f;
    
    return ndc;
}

float4 DepthToWorldPos(float4 ndc)
{
    float4 worldPos = mul(ndc, gInvCurrViewProj);
    
    return worldPos / worldPos.w;
}

float4 MotionBlurPS(PSInput input) : SV_TARGET
{
    float4 scene = ColorTex.Sample(LinearClamp, input.uv);

    // 画面と同じ大きさなので、ピクセル座標でそのまま読める(フィルタを使わない)
    float depth = DepthTex.Load(int3(input.position.xy, 0));
    
    if (gSampleCount <= 1 || gBlurStrength <= 0.0001f)
    {
        return ColorTex.Sample(LinearClamp, input.uv);
    }
 
    float4 clipPos = GetCurrentClipPos(input.uv, depth);
    float4 worldPos = DepthToWorldPos(clipPos);
    
    float4 prevClipPos = mul(worldPos, gPrevViewProj);
    float safeW = (prevClipPos.w >= 0.0f) ? max(prevClipPos.w, 0.0001f) : min(prevClipPos.w, -0.0001f);
    prevClipPos /= safeW;
    
    float2 prevUV = float2(prevClipPos.x * 0.5f + 0.5f, 1.0f - (prevClipPos.y * 0.5f + 0.5f));
    
    float2 velocity = (input.uv - prevUV) * gBlurStrength;
    
    float maxBlur = 0.05f;
    float speed = length(velocity);
    // speedが0でもゼロ除算しないよう max(speed, 0.0001f) で割る
    velocity *= min(speed, maxBlur) / max(speed, 0.0001f);
    
    float4 colorAccum = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float invCount = 1.0f / (float) (gSampleCount - 1);
    
    [loop] //早期リターンがある場合、forがループであると明示的に示す必要がある
    for (int i = 0; i < gSampleCount; ++i)
    {
        // -0.5 ~ +0.5 の範囲で前後均等にサンプリング（または 0 ~ -1 で残像方向のみ）
        float t = (float) i * invCount - 0.5f;
        float2 sampleUV = input.uv + velocity * t;
        colorAccum += ColorTex.Sample(LinearClamp, sampleUV);
    }

    return colorAccum / (float) gSampleCount;
}