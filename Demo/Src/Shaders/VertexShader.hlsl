#include "Common.fx"

struct VertexIn
{
    float3 PosL : SV_POSITION;
    float3 NormalL : NORMAL0;
    float2 Tex : TEXCOORD0;
};  

 
VertexOut main( VertexIn vin ) 
{ 
    VertexOut vout;
    
    vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz;  
    vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
    vout.NormalW = mul(vin.NormalL, (float3x3) gWorldInverseTranspose);
    vout.Tex = vin.Tex;
    
    return vout;
}