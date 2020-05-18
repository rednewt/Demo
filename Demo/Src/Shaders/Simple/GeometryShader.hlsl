#include "Common.hlsli"

[maxvertexcount(3)]
void main(
	point VertexOut input[1], 
	inout LineStream<GSOut> output
)
{
	//[unroll]
	//for (uint i = 0; i < 3; i++)
	//{
 //       GSOut gout;
 //       gout.PosH = mul(input[i].PosL, gWorldViewProj);
		
 //       output.Append(gout);
 //   }
	
    GSOut p;
    p.PosH = mul(input[0].PosL, gWorldViewProj);
    output.Append(p);

    p.PosH = mul(float4(input[0].PosL.xyz + input[0].NormalL, 1.0f), gWorldViewProj);
    output.Append(p);


}