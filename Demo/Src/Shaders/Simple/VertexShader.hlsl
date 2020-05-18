#include "Common.hlsli"


VertexOut main( VertexIn vin )
{
    VertexOut vout;
    
    vout.NormalL = vin.NormalL;
    vout.Tex = vin.Tex;
    vout.PosL = float4(vin.PosL, 1.0f);

    return vout;
}