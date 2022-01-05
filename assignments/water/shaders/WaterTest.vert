#version 330 core
layout (location = 0) in vec3 pos;
uniform mat4 viewProj;
uniform sampler3D heightMap;
uniform float heightMapLayer;
out float place;

void main()
{
   vec4 ndcCoord = viewProj * vec4(pos, 1.0);
   vec3 heightMapCoord = vec3((ndcCoord.x + 1) / 2, (ndcCoord.y + 1) / 2, heightMapLayer);
   float height = texture(heightMap, heightMapCoord).r;
   int heightScale = 3;
   gl_Position = viewProj * vec4(pos.x, pos.y, pos.z, 1.0);
   place = gl_VertexID % 2;
}