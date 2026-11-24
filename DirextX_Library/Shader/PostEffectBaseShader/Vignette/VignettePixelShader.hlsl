Texture2D SceneTex : register(t0); // 入力画像(前段の結果)
SamplerState MainSampler : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

cbuffer VignetteParam
{
    float3 vignetteColor; // ビネットの色 (黒なら float3(0,0,0)、ダメージなら float3(0.6,0,0))
    float intensity; // 暗さ・色の濃さの強さ (標準: 1.0f ~ 1.5f)
    
    float2 centerPos; // ビネットの中心座標 (標準: float2(0.5f, 0.5f))
    float smoothness; // グラデーションの滑らかさ (標準: 0.2f ~ 0.6f)
    float roundness; // 真円度 (0.0f = 画面比率に沿った楕円, 1.0f = 正円)
    
    float aspectRatio; // 画面のアスペクト比 (Width / Height 例: 16.0f / 9.0f)
    float time;
    float2 padding; // 16バイト境界合わせ用パディング
};

//#define NORMALVIGNETTE

float4 VignettePS(PSInput input) : SV_TARGET
{
    float4 color = SceneTex.Sample(MainSampler, input.uv);
    
#ifdef NORMALVIGNETTE

    float2 dist = input.uv - centerPos;
#else
    float2 animatedCenter = centerPos + float2(cos(time * 2.0f), sin(time * 2.5f)) * 0.0f;
    float2 dist = input.uv - animatedCenter;
#endif
    
    dist.x *= lerp(1.0f, aspectRatio, roundness);
    
    float angle = atan2(dist.y, dist.x);
    float wave = sin(angle * 8.0f + time * 4.0f) * 0.03f;
    
    float len = length(dist) + wave;
    
    float vignette = smoothstep(0.8, 0.8 - smoothness, len * intensity);
    
    color.rgb *= vignette;
    
    color.rgb = lerp(vignetteColor, color.rgb, vignette);
    
	return color;
}