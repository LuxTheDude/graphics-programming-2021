#version 330 core
layout (location = 0) in vec3 initialPos;
out float lenColorScale;

uniform mat4 model;
uniform mat4 prevModel;
uniform vec3 offset;
uniform float cubeSize;
uniform vec3 camPos;
uniform vec3 camForward;
uniform vec3 velocity;

float heightScale = 1.1;

void main()
{
	vec3 worldPos = mod(initialPos + offset, cubeSize);
	worldPos += camPos + camForward - (cubeSize / 2.0);
	vec3 prevWorldPos = worldPos - velocity * heightScale;
	vec4 top = model * vec4(prevWorldPos, 1.0);
	vec4 topPrev = prevModel * vec4(prevWorldPos, 1.0);
	vec4 bottom = model * vec4(worldPos, 1.0);
	gl_Position = mix(top, bottom, mod(gl_VertexID, 2));

	vec2 dir = (top.xy/top.w) - (bottom.xy/bottom.w);
	vec2 dirPrev = (topPrev.xy/topPrev.w) - (bottom.xy/bottom.w);

	float len = length(dir);
	float lenPrev = length(dirPrev);

	lenColorScale = clamp(len/lenPrev, 0.0, 1.0);
}