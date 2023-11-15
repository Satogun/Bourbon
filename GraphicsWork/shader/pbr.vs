#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitTangent;

out VS_OUT
{
    vec3 WorldPos;
    vec2 TexCoords;
    vec3 T;
    vec3 N;
}vs_out;


uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{

    gl_Position = projection*view*model* vec4(aPos, 1.0);


    vs_out.TexCoords = aTexCoords;
    vs_out.WorldPos = vec3(model * vec4(aPos, 1.0));

    mat3 nm = transpose(inverse(mat3(model)));


    vs_out.N =normalize( vec3(nm)*aNormal);

    vec3 T = normalize(nm * aTangent);
    vs_out.T = normalize(T - dot(T, vs_out.N) *  vs_out.N);




}