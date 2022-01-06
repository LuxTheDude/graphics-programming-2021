#version 330 core
layout (location = 0) in vec3 pos;

out vec3 texCoord;

uniform mat4 viewProj;

void main()
{
    texCoord = pos;
    vec4 ndcPos = viewProj * vec4(pos, 1.0);
    gl_Position =  ndcPos.xyww;
}  