#version 330 core
layout (location = 0) in vec3 aPos; // the position variable has attribute position 0
layout (location = 1) in vec3 aColor;
  
out vec3 ourColor; // specify a color output to the fragment shader
out vec3 position;
uniform float xOffset;

void main()
{
    position = vec3(aPos.x + xOffset, -aPos.y, aPos.z);
    gl_Position = vec4(position, 1.0); // see how we directly give a vec3 to vec4's constructor
    ourColor = aColor;
}