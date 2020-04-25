
//bare minimum shader for drawing debug objects, such as light location

cbuffer cbConstants : register(b0)
{
    row_major float4x4 gWorldViewProj;
    float4 drawColor;
};

float4 main( float3 pos : POSITION ) : SV_POSITION
{
    return mul(float4(pos, 1.0f), gWorldViewProj);
}