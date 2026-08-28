struct Output
{
    float4 pos : POSITION;
    float4 svpos : SV_Position; //システム用頂点座標
    float2 uv : TEXCOORD; //uv値
};

Texture2D<float4> tex : register(t0); //0番スロットに設定されたTexture
SamplerState smp : register(s0); //０番スロットに設定されたサンプラー