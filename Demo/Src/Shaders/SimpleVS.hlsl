cbuffer cbConstants : register(b0)
{
    row_major float4x4 gWorldViewProj;
    float4 drawColor;
};

struct VertexIn
{
    float3 PosL : SV_POSITION;
    float3 NormalL : NORMAL0;
    float2 Tex : TEXCOORD0;
};


float4 main( VertexIn vin ) : SV_POSITION
{
    return mul(float4(vin.PosL, 1.0f), gWorldViewProj);
}