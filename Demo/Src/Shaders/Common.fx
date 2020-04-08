
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

struct PointLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    
    float3 Position;
    float Range;
    
    float3 Attenuation;
    float pad;
};

struct SpotLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    
    float3 Direction;
    float SpotPower;
    
    float3 Position;
    float Range;
    
    float3 Attenuation;
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

LightingOutput ComputeDirectionalLight(Material mat, DirectionalLight light, float3 normal, float3 toEyeVector)
{
    LightingOutput result;
    
    float diffuseIntensity = max(dot(-light.Direction, normal), 0);
    
    float3 reflectVector = normalize(reflect(light.Direction, normal));   
    float specularIntensity = pow(max(dot(reflectVector, toEyeVector), 0.0f), mat.Specular.w);
    
    result.Ambient = mat.Ambient * light.Ambient;
    result.Diffuse = diffuseIntensity * (mat.Diffuse * light.Diffuse);
    result.Specular = specularIntensity * (mat.Specular * light.Specular);

    return result;
}


LightingOutput ComputePointLight(Material mat, PointLight light, float3 normal, float3 pixelPos, float3 toEyeVector)
{
    float3 lightPosToPixelVector = light.Position - pixelPos;
    float vectorLength = length(lightPosToPixelVector);
    
    LightingOutput result;
    
    result.Ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    result.Diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    result.Specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    if (vectorLength > light.Range)
        return result;
    
    //normalize the vector ourselves since we already computed length
    lightPosToPixelVector /= vectorLength;
    
    float diffuseIntensity = max(dot(lightPosToPixelVector, normal), 0.0f);
    
    float3 reflectVector = reflect(-lightPosToPixelVector, normal);
    float specularIntensity = pow(max(dot(reflectVector, toEyeVector), 0.0f), mat.Specular.w);
    
    //attenuation equation
    //  1 / ( a0 + a1*d + a2* d^2 )
    float att = 1.0f / dot(light.Attenuation, float3(1.0f, vectorLength, vectorLength * vectorLength));

    result.Diffuse = diffuseIntensity * (mat.Diffuse * light.Diffuse) * att;
    result.Specular = specularIntensity * (mat.Specular * light.Specular) * att;
    result.Ambient = mat.Ambient * light.Ambient;
    
    return result;
}

LightingOutput ComputeSpotLight(Material mat, SpotLight light, float3 normal, float3 pixelPos, float3 toEyeVector)
{
    float3 lightPosToPixelVector = light.Position - pixelPos;
    float vectorLength = length(lightPosToPixelVector);
    
    LightingOutput result;
    
    result.Ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    result.Diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    result.Specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    if (vectorLength > light.Range)
        return result;
    
    //normalize the vector ourselves since we already computed length
    lightPosToPixelVector /= vectorLength;
    
    float diffuseIntensity = max(dot(lightPosToPixelVector, normal), 0.0f);
    
    float3 reflectVector = reflect(-lightPosToPixelVector, normal);
    float specularIntensity = pow(max(dot(reflectVector, toEyeVector), 0.0f), mat.Specular.w);
    
    float coneSpotIntensity = pow(max(dot(-light.Direction, lightPosToPixelVector), 0.0f), light.SpotPower);
    //attenuation equation
    // 1 / ( a0 + a1*d + a2* d^2 )
    float att = 1.0f / dot(light.Attenuation, float3(1.0f, vectorLength, vectorLength * vectorLength));
    
    result.Diffuse = diffuseIntensity * (mat.Diffuse * light.Diffuse) * att * coneSpotIntensity;
    result.Specular = specularIntensity * (mat.Specular * light.Specular) * att * coneSpotIntensity;
    result.Ambient = mat.Ambient * light.Ambient * coneSpotIntensity;
    
    return result;
}