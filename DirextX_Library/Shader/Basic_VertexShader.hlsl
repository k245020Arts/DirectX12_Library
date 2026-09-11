#include "BasicShaderHeader.hlsli"

Output BasicVS(float4 pos : POSITION, float2 uv : TEXCOORD, float4 normal : NORMAL)
{
    Output output;
    output.pos = pos;
    output.uv = uv;
    output.pos = mul(mat, pos);
    output.normal = normal;
    return output;
}
