struct VertexIn
{
    float3 PosL : SV_POSITION;
    float3 NormalL : NORMAL0;
    float2 Tex : TEXCOORD0;
};

struct VertexOut
{
    float4 PosL : SV_POSITION;
    float3 NormalL : NORMAL0;
    float2 Tex : TEXCOORD0;
};


struct GSOut
{
    float4 PosH : SV_POSITION;
};

cbuffer cbConstants : register(b0)
{
    row_major float4x4 gWorldViewProj;
    float4 drawColor;
};