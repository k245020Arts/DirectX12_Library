Texture2D SceneTex : register(t0); // 入力画像(前段の結果)
Texture2D<float> DepthTex : register(t1); // シーンの深度(R32_FLOAT)
SamplerState MainSampler : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

// C++ の FogParam と同じ並び
cbuffer FogParam : register(b0)
{
    float4 fogColor;

    float3 cameraPos;
    float density;

    float heightFalloff;
    float baseHeight;
    float maxOpacity;
    float startDistance;

    float skyAmount;
    float3 fogPad;

    float4x4 invViewProj;
};

float3 DepthToWorldPos(float2 uv, float depth)
{
    float4 ndc;
    ndc.x = uv.x * 2.0f - 1.0f;
    ndc.y = (1.0f - uv.y) * 2.0f - 1.0f;
    ndc.z = depth;
    ndc.w = 1.0f;
    
    float4 worldPos = mul(ndc, invViewProj);
    
    return worldPos.xyz / worldPos.w;
}

float4 FogPS(PSInput input) : SV_TARGET
{
    float4 scene = SceneTex.Sample(MainSampler, input.uv);

    // 画面と同じ大きさなので、ピクセル座標でそのまま読める(フィルタを使わない)
    float depth = DepthTex.Load(int3(input.position.xy, 0));
 
    float3 worldPos = DepthToWorldPos(input.uv, depth);

    float3 toPoint = worldPos - cameraPos;

    float dist = max(length(toPoint), 0.0001f);

    float segment = max(dist - startDistance, 0.0f);
    float startDist = dist - segment;

    float slope = toPoint.y / dist;
    float startHeight = cameraPos.y + slope * startDist;

    float densityAtStart = density * exp(clamp(-heightFalloff * (startHeight - baseHeight), -80.0f, 80.0f));

    float x = heightFalloff * slope * segment;
    float xs = x + step(abs(x), 0.0001f) * 0.0001f;
    float integral = (1.0f - exp(clamp(-xs, -80.0f, 80.0f))) / xs;

    float tau = densityAtStart * segment * integral;
    float fog = min(1.0f - exp(-tau), maxOpacity);

    fog *= lerp(1.0f, skyAmount, step(0.99999f, depth));

    scene.rgb = lerp(scene.rgb, fogColor.rgb, fog);

    return scene;
}
