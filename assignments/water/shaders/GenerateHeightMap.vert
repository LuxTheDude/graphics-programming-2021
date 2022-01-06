#version 330 core
layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 uv;

smooth out vec2 worldPos;

void main()
{
   worldPos = pos;
   gl_Position = vec4(uv, 0, 1);
}
