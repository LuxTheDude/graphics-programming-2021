#version 330 core
out vec4 FragColor;
in  vec3 heightMapCoord;
uniform sampler3D heightMap;
void main()
{
   FragColor = vec4(vec3(texture(heightMap, heightMapCoord).r), 1);
}