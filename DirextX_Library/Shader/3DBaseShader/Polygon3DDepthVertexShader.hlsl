cbuffer Transform : register(b0)
{
    float4x4 World;
    float4x4 View;
    float4x4 Proj;
};

struct VSInput
{
    float3 pos : POSITION;
};

float4 DepthVS_Polygon3D(VSInput input) : SV_POSITION
{
    float4 localPos = float4(input.pos, 1.0f);

    float4 worldPos = mul(World, localPos);
    float4 viewPos = mul(View, worldPos);
    float4 projPos = mul(Proj, viewPos);

    return projPos;
}