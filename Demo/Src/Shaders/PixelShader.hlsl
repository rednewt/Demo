#include "Common.fx"

//register b0 is already used by cbPerObject from Common.fx

cbuffer cbPerFrame : register(b1)
{
    DirectionalLight gLight;
    PointLight gPointLight;
    float3 gEyePos; //In world space
    float pad;
};

 
SamplerState Sampler : register(s0);
Texture2D DiffuseMap : register(t0); 

float4 main(VertexOut pin) : SV_TARGET
{
    pin.NormalW = normalize(pin.NormalW);
    
    float3 ToEyeVector = normalize(gEyePos - pin.PosW);
    
    LightingOutput results[2];
    results[0] = ComputeDirectionalLight(gMaterial, gLight, pin.NormalW, ToEyeVector);
    results[1] = ComputePointLight(gMaterial, gPointLight, pin.NormalW, pin.PosW, ToEyeVector);
    
    LightingOutput result;
    result.Ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    result.Diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    result.Specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    [unroll]
    for (int i = 0; i < 2; ++i)
    {
        result.Ambient += results[i].Ambient;
        result.Diffuse += results[i].Diffuse;
        result.Specular += results[i].Specular;
    }
    
    float4 SampleColor = DiffuseMap.Sample(Sampler, pin.Tex);
    float4 final = SampleColor * (result.Ambient + result.Diffuse) + result.Specular;
    final.w = 1.0f;
    
    return final;
}