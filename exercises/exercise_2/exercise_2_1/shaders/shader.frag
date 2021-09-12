#version 330 core
out vec4 FragColor;
  
//in vec4 vertexColor; // the input variable from the vertex shader (same name and same type)  
in vec3 ourColor;
in vec3 position;

void main()
{
    FragColor = vec4(position, 1.0);
} 