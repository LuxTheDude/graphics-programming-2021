#version 330 core
layout (location = 0) in vec3 initialPos;

uniform mat4 model;
uniform mat4 prevModel;
uniform vec3 offset;
uniform float cubeSize;
uniform vec3 camPos;
uniform vec3 camForward;

void main()
{
	vec3 worldPos = mod(initialPos + offset, cubeSize);
	worldPos += camPos + camForward - (cubeSize / 2.0);
	gl_Position = model * vec4(worldPos, 1.0);
	float dist = distance(vec3(0), gl_Position.xyz);
	gl_PointSize = min(100.0/dist, 5);
}