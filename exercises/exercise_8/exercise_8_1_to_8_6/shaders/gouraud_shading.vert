#version 330 core
layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textCoord; // here for completness, but we are not using it just yet

uniform mat4 model; // represents model coordinates in the world coord space
uniform mat4 invTransposeModel; // inverse of the transpose of  model (used to multiply vectors while preserving angles)
uniform mat4 view;  // represents the world coordinates in the camera coord space
uniform mat4 projection; // camera projection matrix
uniform vec3 camPosition; // so we can compute the view vector (could be extracted from view matrix, but let's make our life easier :) )

// send shaded color to the fragment shader
out vec4 shadedColor;

// TODO exercise 8 setup the uniform variables needed for lighting
// light uniform variables
uniform vec3 ambientColor;
uniform vec3 light1Pos;
uniform vec3 light1Color;
// material properties
uniform vec3 reflectionColor;
uniform float ambientReflectance;
uniform float diffuseReflectance;
uniform float specularReflectance;
uniform float specularExponent;
// attenuation uniforms


void main() {
   // vertex in world space (for light computation)
   vec4 P = model * vec4(vertex, 1.0);
   // normal in world space (for light computation)
   vec3 N = normalize((invTransposeModel * vec4(normal, 0.0)).xyz);

   // final vertex transform (for opengl rendering, not for lighting)
   gl_Position = projection * view * P;

   // TODO exercises 8.1, 8.2 and 8.3 - Gouraud shading (i.e. Phong reflection model computed in the vertex shader)
   vec3 L1 = normalize(light1Pos - P.xyz);
   vec3 V = normalize(camPosition - P.xyz);
   vec3 H1 = normalize(L1 + V);
   // TODO 8.1 ambient
   vec3 ambient = ambientColor * reflectionColor * ambientReflectance;
   // TODO 8.2 diffuse
   vec3 diffuse = light1Color * reflectionColor * diffuseReflectance * dot(N, L1);
   // TODO 8.3 specular
   vec3 specular = light1Color * specularReflectance * pow(dot(N, H1), specularExponent);
   // TODO exercise 8.6 - attenuation - light 1


   // TODO set the output color to the shaded color that you have computed
   shadedColor = vec4(ambient + diffuse + specular, 1.0);

}