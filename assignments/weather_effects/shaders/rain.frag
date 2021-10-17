#version 330 core
in float lenColorScale;
out vec4 fragColor;

void main()
{
    fragColor = vec4(vec3(0.0117, 0.2901, 0.9254),lenColorScale);
}