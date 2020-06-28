
struct Data
{
    float4 v1;
    float3 v2;
};
//512 x 512

StructuredBuffer<Data> gInputA : register(t0);
StructuredBuffer<Data> gInputB : register(t1);

RWStructuredBuffer<Data> gOutput : register(u0);

[numthreads(256, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    gOutput[DTid.x].v1 = gInputA[DTid.x].v1 * gInputB[DTid.x].v1;
    gOutput[DTid.x].v2 = gInputA[DTid.x].v2 * gInputB[DTid.x].v2;
    
}