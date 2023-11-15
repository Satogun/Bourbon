#version 430 core
out vec4 FragColor;

in VS_OUT
{
    vec3 WorldPos;
    vec2 TexCoords;
    vec3 T;
    vec3 N;
}fs_in;

struct DirLight
{
    vec3 direction;
    vec3 color;
};
uniform DirLight dirLight;


// material parameters
uniform sampler2D albedoMap;
uniform sampler2D emissiveMap;
uniform sampler2D normalsMap;
uniform sampler2D metalRoughMap;
uniform sampler2D aoMap;

uniform bool normalMapped;
uniform bool aoMapped;
uniform bool IBL;

// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;


uniform vec3 camPos;

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

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalsMap, fs_in.TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(fs_in.WorldPos);
    vec3 Q2  = dFdy(fs_in.WorldPos);
    vec2 st1 = dFdx(fs_in.TexCoords);
    vec2 st2 = dFdy(fs_in.TexCoords);

    vec3 N   = normalize(fs_in.N);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

vec3 calcDirLight(DirLight light,vec3 N,vec3 V,vec3 albedo, float rough, float metal, vec3 F0 );

void main()
{		
    // material properties
    vec4 color = texture(albedoMap, fs_in.TexCoords).rgba;
    vec3 emissive = texture(emissiveMap,fs_in.TexCoords).rgb;
    vec2 metalRough =  texture(metalRoughMap, fs_in.TexCoords).gb;
    float metallic = metalRough.y;
    float roughness = metalRough.x;
    float ao = texture(aoMap, fs_in.TexCoords).r;

    vec3 albedo = color.rgb;
    
       
    vec3 N = vec3(0.0f);
    if(normalMapped)
    {
        N = getNormalFromMap();
    }
    else
    {
        N = normalize(fs_in.N);
    }

    vec3 V = normalize(camPos - fs_in.WorldPos);
    vec3 R = reflect(V, N); 

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
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
        ambient = (kD * diffuse + specular);

    }

    if(aoMapped)
    {
        ambient *=ao;
    }
    
    
    radianceOut += ambient;
    radianceOut += emissive;
    

    // gamma correct
    radianceOut = pow(radianceOut, vec3(1.0/2.2)); 

    FragColor = vec4(radianceOut,1.0f);
}



vec3 calcDirLight(DirLight light,vec3 N,vec3 V,vec3 albedo, float rough, float metal, vec3 F0 )
{
    vec3 L = normalize(-dirLight.direction);
    vec3 H = normalize(L + V);
    float NdV = max(dot(N,V),0.0f);
    float NdL = max(dot(N,L),0.0f);
    vec3 radianceIn = dirLight.color;

    float NDF = DistributionGGX(N, H, rough);
    float G   = GeometrySmith(NdV, NdL, rough);
    vec3  F   = fresnelSchlick(max(dot(H,V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metal;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * NdV * NdL;
    vec3 specular = numerator / max (denominator, 0.0001);

    vec3 radiance = (kD * (albedo / PI) + specular ) * radianceIn * NdL;

    return radiance;

}