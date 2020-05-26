struct VertexIn
{
    float3 CenterW : POSITION;
    float2 Size : SIZE;
};

typedef VertexIn VertexOut;

struct GSOut
{
    float4 PosH : SV_Position;
    uint PrimID : SV_PrimitiveID;
};