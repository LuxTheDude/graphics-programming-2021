#version 330 core
in float lenColorScale;
out vec4 fragColor;

void main()
{
    fragColor = vec4(vec3(1),lenColorScale);
}