
struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : WORLD_POS;
    float3 NormalW : NORMAL0;
    float2 Tex : TEXCOORD0;
};

struct Material
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
};

struct DirectionalLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float3 Direction;
    float pad;
};

cbuffer cbPerObject : register(b0)
{
    row_major float4x4 gWorldViewProj;
    row_major float4x4 gWorld;
    row_major float4x4 gWorldInverseTranspose;
    Material gMaterial;
};

typedef Material LightingOutput;

LightingOutput ComputeDirectionalLight(Material mat, DirectionalLight light, float3 normal, float3 ToEyeVector)
{
    LightingOutput result;
    
    result.Ambient = mat.Ambient * light.Ambient;
    
    float diffuseIntensity = max(dot(-light.Direction, normal), 0);
    result.Diffuse = diffuseIntensity * (mat.Diffuse * light.Diffuse);
    
    
    float3 reflectVector = normalize(reflect(light.Direction, normal));
    float specularIntensity = pow(max(dot(reflectVector, ToEyeVector), 0), mat.Specular.w);
    
    result.Specular = specularIntensity * (mat.Specular * light.Specular);

    return result;
}
