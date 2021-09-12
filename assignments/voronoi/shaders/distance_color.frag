#version 330 core
// FRAGMENT SHADER

// TODO voronoi 1.5
out vec4 fragColor;
// Create an 'in float' variable to receive the depth value from the vertex shader,
// the variable must have the same name as the 'out variable' in the vertex shader.
// CODE HERE
in float zCoord;
// Create a 'uniform vec3' to receive the cone color from your application.
// CODE HERE
uniform vec3 color;

void main()
{
    // Modulate the color of the fragColor using the z-coordinate of the fragment.
    // Make sure that the z-coordinate is in the [0, 1] range (if it is not, place it in that range),
    // you can use non-linear transformations of the z-coordinate, such as the 'pow' or 'sqrt' functions,
    // to make the colors brighter close to the center of the cone.
    float clamped = clamp(zCoord, 0.0, 1.0);
    float brightness = pow(clamped, 8);
    vec3 modColor = mix(color, vec3(1.0), brightness);
    fragColor = vec4(modColor, 1.0); // CODE HERE
}