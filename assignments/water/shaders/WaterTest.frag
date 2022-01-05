#version 330 core
out vec4 FragColor;
in float place;
in float height;
in vec3 color;
void main()
{
   FragColor = vec4(color, 1);
}