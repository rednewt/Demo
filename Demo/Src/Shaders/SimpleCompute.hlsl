
struct Data
{
    float4 v1;
    float3 v2;
};
//512 x 512

ConsumeStructuredBuffer<Data> gInput : register(u0);
AppendStructuredBuffer<Data> gOutput : register(u1);

[numthreads(256, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    Data d = gInput.Consume();
   
    d.v1 *= 2;
    d.v2 *= 2;
    
    gOutput.Append(d);

}