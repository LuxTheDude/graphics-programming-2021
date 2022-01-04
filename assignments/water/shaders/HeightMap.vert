#version 330 core
layout (location = 0) in vec3 pos;
uniform float heightMapLayer;
out vec3 heightMapCoord;

void main()
{
   gl_Position = vec4(pos, 1.0);
   heightMapCoord = vec3((pos.x + 1) / 2, (pos.y + 1) / 2, heightMapLayer);
}