#version 330 core
layout (location = 0) in vec3 pos;
out vec4 vtxColor;
out vec3 outPos;

void main()
{
   gl_Position = vec4(pos, 1.0);
   outPos = pos;
   vtxColor = vec4(0.5, 0.5, 0.5, 1);
}