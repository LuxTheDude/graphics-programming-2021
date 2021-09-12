#version 330 core
layout (location = 0) in vec2 pos;   // the position variable has attribute position 0
// TODO 2.2 add velocity and timeOfBirth as vertex attributes
layout (location = 1) in vec2 velocity;
layout (location = 2) in float timeOfBirth;
out float age;

// TODO 2.3 create and use a float uniform for currentTime
uniform float currentTime;

// TODO 2.6 create out variable to send the age of the particle to the fragment shader

void main()
{
    float maxAge = 10;
    // TODO 2.3 use the currentTime to control the particle in different stages of its lifetime
    gl_Position = vec4(3.0, 3.0, 3.0, 1.0);
    age = 0;
    if(timeOfBirth > 0)
    {
        age = currentTime - timeOfBirth;
        if(age <= maxAge)
            gl_Position = vec4(pos + (age * velocity), 0.0, 1.0);
    }

    float size = mix(0.1, 20, age / maxAge);
        
    // TODO 2.6 send the age of the particle to the fragment shader using the out variable you have created

    // this is the output position and and point size (this time we are rendering points, instad of triangles!)
    gl_PointSize = size;
}