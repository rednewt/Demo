
struct VertexOut
{
    float4 PosH : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

SamplerState SamState : register(s0);
Texture2D DiffuseMap : register(t0);

float4 main(VertexOut pin) : SV_TARGET
{
    return DiffuseMap.Sample(SamState, pin.Tex);
}