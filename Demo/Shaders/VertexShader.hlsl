cbuffer cbPerObject
{
    row_major float4x4 WorldViewProj;
};

struct VertexIn
{
    float3 PosL : POSITION;
    float4 Color : COLOR;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float4 Color : COLOR;
};

VertexOut main( VertexIn vin ) 
{
    VertexOut vout;

    vout.PosH = mul(float4(vin.PosL, 1.0f), WorldViewProj);
    //vout.PosH = float4(vin.PosL, 1.0f);
    vout.Color = vin.Color;
    
    return vout;
}