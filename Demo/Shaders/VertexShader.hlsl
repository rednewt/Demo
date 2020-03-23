cbuffer cbPerObject
{
    row_major float4x4 WorldViewProj;
};

struct VertexIn
{
    float3 PosL : POSITION;
    float2 Tex : TEXCOORD0;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

VertexOut main( VertexIn vin ) 
{
    VertexOut vout;

    vout.PosH = mul(float4(vin.PosL, 1.0f), WorldViewProj);
    vout.Tex = vin.Tex;
    
    return vout;
}