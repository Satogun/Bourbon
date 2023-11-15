#version 430 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D albedoMap;
uniform sampler2D emissiveMap;
uniform sampler2D normalsMap;
uniform sampler2D lightMap;
uniform sampler2D metalRoughMap;

void main()
{    
    FragColor = texture(lightMap, TexCoords);
}