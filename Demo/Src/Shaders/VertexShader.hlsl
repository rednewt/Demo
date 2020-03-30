cbuffer cbPerObject : register(b0)
{
    row_major float4x4 WorldViewProj;
    row_major float4x4 World;
    row_major float4x4 WorldInverseTranspose;
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
    float3 PosW : WORLD_POS;
    float3 NormalW : NORMAL0;
    float2 Tex : TEXCOORD0;
};

VertexOut main( VertexIn vin ) 
{
    VertexOut vout;
    
    vout.PosW = mul(float4(vin.PosL, 1.0f), World).xyz;
    vout.PosH = mul(float4(vin.PosL, 1.0f), WorldViewProj);
    vout.NormalW = mul(vin.NormalL, (float3x3) WorldInverseTranspose);
    vout.Tex = vin.Tex;
    
    return vout;
}