#version 330 core
smooth in vec2 worldPos;
uniform sampler3D heightMap;
uniform vec2 screenSize;
uniform float heightMapLayer;

out vec3 FragColor;

void main()
{
   vec2 uv = gl_FragCoord.xy / screenSize;
   vec3 heightMapCoord = vec3(worldPos, 0);
   float height = texture(heightMap, heightMapCoord).r;
   FragColor = vec3(worldPos, height);
   
}