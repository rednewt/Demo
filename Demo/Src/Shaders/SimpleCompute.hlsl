

//512 x 512
Texture2D gInputA : register(t0);
Texture2D gInputB : register(t1);

RWTexture2D<float4> gOutput : register(u0);

[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    gOutput[DTid.xy] = gInputA[DTid.xy] * gInputB[DTid.xy];
}