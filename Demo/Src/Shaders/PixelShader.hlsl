

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : WORLD_POS;
    float3 NormalW : NORMAL0;
    float2 Tex : TEXCOORD0;
};


struct DirectionalLight
{
    float3 Direction;
    float pad;
};

cbuffer cbPerFrame : register(b0)
{
    DirectionalLight gLight;
};

SamplerState SamState : register(s0);
Texture2D DiffuseMap : register(t0); 

float4 main(VertexOut pin) : SV_TARGET
{
    float4 SampleColor = DiffuseMap.Sample(SamState, pin.Tex);
    
    pin.NormalW = normalize(pin.NormalW);
    
    //Expects Direction Vector to be normalized 
    float3 LightVector = -gLight.Direction;
    float Intensity = max(dot(LightVector, pin.NormalW), 0.0f);
    
    float4 Ambient = 0.3f * SampleColor;
    float4 Diffuse = Intensity * SampleColor;
    
    return Diffuse + Ambient;
}