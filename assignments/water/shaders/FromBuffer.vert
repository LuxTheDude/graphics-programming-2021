#version 330 core
layout (location = 0) in vec3 pos;
out vec4 vtxColor;

void main()
{
   gl_Position = vec4(pos, 1.0) + vec4(vec3(0.5), 0.0);
   vtxColor = vec4(1, 1, 1, 1);
}