cbuffer cbPerObject
{
    row_major float4x4 WorldViewProj;
};

struct VertexIn
{
    float3 PosL : SV_POSITION;
    float3 NormalL : NORMAL0;
    float2 Tex : TEXCOORD0;
}; 

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 NormalW : NORMAL0;
    float2 Tex : TEXCOORD0;
};

VertexOut main( VertexIn vin ) 
{
    VertexOut vout;

    vout.PosH = mul(float4(vin.PosL, 1.0f), WorldViewProj);
    vout.NormalW = vin.NormalL;
    vout.Tex = vin.Tex;
    
    return vout;
}