#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texCoord;
uniform mat4 viewProj;
uniform sampler3D heightMap;
uniform sampler2D normalMap;
uniform float heightMapLayer;
out float place;
out float height;
out vec3 color;

void main()
{
   //vec3 heightMapCoord = vec3(texCoord, heightMapLayer);
   vec3 heightMapCoord = vec3(pos.xz, 0);
   height = texture(heightMap, heightMapCoord).r;
   int heightScale = 10;
   color = texture(normalMap, texCoord).rgb;
   gl_Position = viewProj * vec4(pos.x, pos.y, pos.z, 1.0);
   place = gl_VertexID % 2;
}