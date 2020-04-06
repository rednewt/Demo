#include "Common.fx"

//register b0 is already used by common cbPerObject

cbuffer cbPerFrame : register(b1)
{
    DirectionalLight gLight;
    float3 gEyePos; //world space
    float pad;
};


SamplerState SamState : register(s0);
Texture2D DiffuseMap : register(t0); 

float4 main(VertexOut pin) : SV_TARGET
{
    pin.NormalW = normalize(pin.NormalW);
    
    float3 ToEyeVector = normalize(gEyePos - pin.PosW);
    LightingOutput result = ComputeDirectionalLight(gMaterial, gLight, pin.NormalW, ToEyeVector);
    
    float4 SampleColor = DiffuseMap.Sample(SamState, pin.Tex);
    float4 final = SampleColor * (result.Ambient + result.Diffuse) + result.Specular;
      
    return final;
}