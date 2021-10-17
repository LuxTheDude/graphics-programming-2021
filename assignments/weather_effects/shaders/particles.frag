#version 330 core
out vec4 fragColor;

void main()
{
    vec2 center = vec2(0.5, 0.5);
    float dist = distance(center, gl_PointCoord);
    float alpha = mix(1.0, 0.0, dist*2);
    fragColor = vec4(vec3(1), alpha);
}