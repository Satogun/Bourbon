#version 430 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gEmissive;
uniform sampler2D gMetalRoughtAO;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

uniform vec3 camPos;
uniform bool IBL;

#define  PI 3.1415926535897932384626433832795
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(float NdV, float NdL, float roughness)
{
    float ggx2 = GeometrySchlickGGX(NdV, roughness);
    float ggx1 = GeometrySchlickGGX(NdL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    float val = 1.0 - cosTheta;
    return F0 + (1.0 - F0) * (val*val*val*val*val); //Faster than pow
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    float val = 1.0 - cosTheta;
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * (val*val*val*val*val); //Faster than pow
}   
// ----------------------------------------------------------------------------



struct DirLight
{
    vec3 direction;
    vec3 color;
};
uniform DirLight dirLight;

void main()
{             
    vec3 WorldPos = texture(gPosition,TexCoords).rgb;
    vec3 N = texture(gNormal,TexCoords).rgb;
    vec3 albedo = texture(gAlbedo,TexCoords).rgb;
    vec3 emissive = texture(gEmissive,TexCoords).rgb;
    vec3 mra = texture(gMetalRoughtAO,TexCoords).rgb;
    float metallic = mra.x;
    float roughness = mra.y;
    float ao = mra.z;

    vec3 V = normalize(camPos - WorldPos);
    vec3 R = reflect(V, N); 

        vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 radianceOut = vec3(0.0);


    vec3 ambient = vec3(0.025)* albedo;

    if(IBL)
     {
        vec3  kS = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
        vec3  kD = 1.0 - kS;
        kD *= 1.0 - metallic;
        vec3 irradiance = texture(irradianceMap, N).rgb;
        vec3 diffuse    = irradiance * albedo;

        const float MAX_REFLECTION_LOD = 4.0;
        vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
        vec2 envBRDF = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
        vec3 specular = prefilteredColor * (kS * envBRDF.x + envBRDF.y);
        ambient = (kD * diffuse + specular)*ao;
    }

    radianceOut += ambient;
    radianceOut += emissive;
    

    float brightness = dot(radianceOut, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 0.8f)
        BrightColor = vec4(radianceOut, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
    FragColor = vec4(radianceOut, 1.0);

}