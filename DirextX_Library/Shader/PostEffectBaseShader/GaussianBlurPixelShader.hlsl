Texture2D MainTex : register(t0);
SamplerState MainSampler : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

cbuffer GaussianBlurParam : register(b0)
{
    float4 bkWeights[2];
    float2 direction;
    float offsetScale;
};

float4 GaussianBlurPSShader(PSInput input) : SV_TARGET
{
    float4 color = MainTex.Sample(MainSampler, input.uv);
    
    //モノクロ
    //float Y = dot(color.rgb, float3(0.299, 0.587, 0.114));
    //float morokuro = float4(Y, Y, Y, 1);
    //return morokuro;
    
    //反転
    //float reverceColor = float4(1.0f - color.rgb, color.a);
    
    //ぼかし
    float w, h, levels;
    MainTex.GetDimensions(0, w, h, levels);
    
    float dx = 1.0 / w;
    float dy = 1.0 / h;
    float4 ret = float4(0, 0, 0, 0);
    
    ///基本的なぼかし(平均値を使用)
    
    //ret += MainTex.Sample(MainSampler, input.uv + float2(-2 * dx, -2 * dy)); //左上
    //ret += MainTex.Sample(MainSampler, input.uv + float2(0, -2 * dy)); //上
    //ret += MainTex.Sample(MainSampler, input.uv + float2(2 * dx, -2 * dy)); //右上
    
    //ret += MainTex.Sample(MainSampler, input.uv + float2(-2 * dx, 0)); //左
    //ret += MainTex.Sample(MainSampler, input.uv);
    //ret += MainTex.Sample(MainSampler, input.uv + float2(2 * dx, 0)); //右
    
    //ret += MainTex.Sample(MainSampler, input.uv + float2(-2 * dx, 2 * dy)); //左下
    //ret += MainTex.Sample(MainSampler, input.uv + float2(0, 2 * dy)); //下
    //ret += MainTex.Sample(MainSampler, input.uv + float2(2 * dx, 2 * dy)); //右下
    
    //return ret / 9.0f;
    
    //エンボス加工
    //ret += MainTex.Sample(MainSampler, input.uv + float2(-2 * dx, -2 * dy)) * 2; //左上
    //ret += MainTex.Sample(MainSampler, input.uv + float2(0, -2 * dy)); //左上
    //ret += MainTex.Sample(MainSampler, input.uv + float2(2 * dx, -2 * dy)) * 0; //右上
    
    //ret += MainTex.Sample(MainSampler, input.uv + float2(-2 * dx, 0)); //左
    //ret += MainTex.Sample(MainSampler, input.uv);
    //ret += MainTex.Sample(MainSampler, input.uv + float2(2 * dx, 0)) * -1; //右
    
    //ret += MainTex.Sample(MainSampler, input.uv + float2(-2 * dx, 2 * dy)) * 0; //左下
    //ret += MainTex.Sample(MainSampler, input.uv + float2(0, 2 * dy)) * -1; //下
    //ret += MainTex.Sample(MainSampler, input.uv + float2(2 * dx, 2 * dy)) * -2; //右下
    
    //float Y = dot(ret.rgb, float3(0.299, 0.587, 0.114));
    //float morokuro = float4(Y, Y, Y, 1);
    //return morokuro;
    
    //シャープネス
    //ret += MainTex.Sample(MainSampler, input.uv + float2(-2 * dx, -2 * dy)) * 0; //左上
    //ret += MainTex.Sample(MainSampler, input.uv + float2(0, -2 * dy)) * -1; //上
    //ret += MainTex.Sample(MainSampler, input.uv + float2(2 * dx, -2 * dy)) * 0; //右上
    
    //ret += MainTex.Sample(MainSampler, input.uv + float2(-2 * dx, 0)) * -1; //左
    //ret += MainTex.Sample(MainSampler, input.uv) * 5;
    //ret += MainTex.Sample(MainSampler, input.uv + float2(2 * dx, 0)) * -1; //右
    
    //ret += MainTex.Sample(MainSampler, input.uv + float2(-2 * dx, 2 * dy)) * 0; //左下
    //ret += MainTex.Sample(MainSampler, input.uv + float2(0, 2 * dy)) * -1; //下
    //ret += MainTex.Sample(MainSampler, input.uv + float2(2 * dx, 2 * dy)) * 0; //右下
    //return saturate(ret);
    
    //輪郭線抽出
    //ret += MainTex.Sample(MainSampler, input.uv + float2(0, -2 * dy)) * -1; //上
    //ret += MainTex.Sample(MainSampler, input.uv + float2(-2 * dx, 0)) * -1; //左
    //ret += MainTex.Sample(MainSampler, input.uv) * 4;
    //ret += MainTex.Sample(MainSampler, input.uv + float2(2 * dx, 0)) * -1; //右
    //ret += MainTex.Sample(MainSampler, input.uv + float2(0, 2 * dy)) * -1; //下
    
    //float Y = dot(ret.rgb, float3(0.299, 0.587, 0.114));
    //Y = pow(1.0f - Y, 10.0f);
    //Y = step(0.2, Y);
    //float morokuro = float4(Y, Y, Y, color.a);
    
    //return morokuro;
    
    //簡易版ガウシアンぼかし
    
    //ret += MainTex.Sample(MainSampler, input.uv + float2(-2 * dx, 2 * dy)) * 1 / 256;
    //ret += MainTex.Sample(MainSampler, input.uv + float2(-1 * dx, 2 * dy)) * 4 / 256;
    //ret += MainTex.Sample(MainSampler, input.uv + float2(0 * dx, 2 * dy)) * 6 / 256;
    //ret += MainTex.Sample(MainSampler, input.uv + float2(1 * dx, 2 * dy)) * 4 / 256;
    //ret += MainTex.Sample(MainSampler, input.uv + float2(2 * dx, 2 * dy)) * 1 / 256;
    
    //ret += MainTex.Sample(MainSampler, input.uv + float2(-2 * dx, 1 * dy)) * 4 / 256;
    //ret += MainTex.Sample(MainSampler, input.uv + float2(-1 * dx, 1 * dy)) * 16 / 256;
    //ret += MainTex.Sample(MainSampler, input.uv + float2(0 * dx, 1 * dy)) * 24 / 256;
    //ret += MainTex.Sample(MainSampler, input.uv + float2(1 * dx, 1 * dy)) * 16 / 256;
    //ret += MainTex.Sample(MainSampler, input.uv + float2(2 * dx, 1 * dy)) * 4 / 256;
    
    //ret += MainTex.Sample(MainSampler, input.uv + float2(-2 * dx,0 * dy)) * 6 / 256;
    //ret += MainTex.Sample(MainSampler, input.uv + float2(-1 * dx,0 * dy)) * 24 / 256;
    //ret += MainTex.Sample(MainSampler, input.uv + float2(0 * dx, 0 * dy)) * 36 / 256;
    //ret += MainTex.Sample(MainSampler, input.uv + float2(1 * dx, 0 * dy)) * 24 / 256;
    //ret += MainTex.Sample(MainSampler, input.uv + float2(2 * dx, 0 * dy)) * 6 / 256;
    
    //ret += MainTex.Sample(MainSampler, input.uv + float2(-2 * dx,-1 * dy)) * 4 / 256;
    //ret += MainTex.Sample(MainSampler, input.uv + float2(-1 * dx,-1 * dy)) * 16 / 256;
    //ret += MainTex.Sample(MainSampler, input.uv + float2(0 * dx, -1 * dy)) * 24 / 256;
    //ret += MainTex.Sample(MainSampler, input.uv + float2(1 * dx, -1 * dy)) * 16 / 256;
    //ret += MainTex.Sample(MainSampler, input.uv + float2(2 * dx, -1 * dy)) * 4 / 256;
    
    //ret += MainTex.Sample(MainSampler, input.uv + float2(-2 * dx, -2 * dy)) * 1 / 256;
    //ret += MainTex.Sample(MainSampler, input.uv + float2(-1 * dx, -2 * dy)) * 4 / 256;
    //ret += MainTex.Sample(MainSampler, input.uv + float2(0 * dx, -2 * dy)) * 6 / 256;
    //ret += MainTex.Sample(MainSampler, input.uv + float2(1 * dx, -2 * dy)) * 4 / 256;
    //ret += MainTex.Sample(MainSampler, input.uv + float2(2 * dx, -2 * dy)) * 1 / 256;
    
    //float[8]だとGPUアライメントの影響を受けてしまうためあえてfloat4[2]にしている
 
    float2 texelSize = float2(1.0f / w, 1.0f / h);
    
    ret += bkWeights[0].x * color;
    for (int i = 1; i < 8; ++i)
    {
        float2 offset = texelSize * direction * i * offsetScale;
        
        ret += bkWeights[i >> 2][i % 4] * MainTex.Sample(MainSampler, input.uv + offset);
        ret += bkWeights[i >> 2][i % 4] * MainTex.Sample(MainSampler, input.uv - offset);
    }
    
    return float4(ret.rgb, color.a);
}