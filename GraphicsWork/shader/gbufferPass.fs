#version 430 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec3 gEmissive;
layout (location = 4) out vec3 gMetalRoughAO;

in VS_OUT
{
    vec3 WorldPos;
    vec2 TexCoords;
    vec3 T;
    vec3 N;
}fs_in;

uniform sampler2D albedoMap;
uniform sampler2D emissiveMap;
uniform sampler2D normalsMap;
uniform sampler2D aoMap;
uniform sampler2D metalRoughMap;


uniform bool normalMapped;
uniform bool aoMapped;


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


void main()
{    
    gPosition = fs_in.WorldPos;
    gAlbedo = texture(albedoMap, fs_in.TexCoords).rgb;
    gEmissive = texture(emissiveMap,fs_in.TexCoords).rgb;

    if(normalMapped)
    {
        gNormal = getNormalFromMap();
    }
    else
    {
        gNormal = normalize(fs_in.N);
    }

    gMetalRoughAO.xy = texture(metalRoughMap,fs_in.TexCoords).bg;
    gMetalRoughAO.z = 1.0f;
    if(aoMapped)
    {
         gMetalRoughAO.z = texture(aoMap,fs_in.TexCoords).r;
    }

}