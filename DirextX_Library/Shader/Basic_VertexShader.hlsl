#include "BasicShaderHeader.hlsli"

Output BasicVS(float4 pos : POSITION, float2 uv : TEXCOORD, float4 normal : NORMAL)
{
    Output output;
    output.uv = uv;
    output.pos = mul(mul(viewproj, world), pos);
    normal.w = 0; //平行成分を無効にする
    output.normal = mul(world,normal); //法線にもワールド変換を行う
 

    return output;
}
