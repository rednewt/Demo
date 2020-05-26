#include "Common.hlsli"

cbuffer cbPerFrame : register(b0)
{
    float3 gEyePosW;
    float pad;
}

[maxvertexcount(4)]
void main(
	point VertexIn input[1],
	inout TriangleStream<GSOut> output,
	uint primID : SV_PrimitiveID)
{
    float3 upVec = float3(0.0f, 1.0f, 0.0f);
    
    float3 forwardVec = gEyePosW - input[0].CenterW;
    forwardVec.y = 0.0f;
    forwardVec = normalize(forwardVec);
    
    float3 rightVec = cross(upVec, forwardVec);
    
    
    
	[unroll]
    for (int i = 0; i < 4; ++i)
    {
		
    }

}