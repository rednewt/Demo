#include "Common.hlsli"

[maxvertexcount(3)]
void main(
	triangle VertexOut input[3], 
	inout TriangleStream<GSOut> output
)
{
	[unroll]
	for (uint i = 0; i < 3; i++)
	{
        GSOut gout;
        gout.PosH = mul(input[i].PosL, gWorldViewProj);
		
        output.Append(gout);
    }
	
}