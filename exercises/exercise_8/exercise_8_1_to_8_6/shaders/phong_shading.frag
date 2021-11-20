#version 330 core

uniform vec3 camPosition; // so we can compute the view vector
out vec4 FragColor; // the output color of this fragment

// TODO exercise 8.4 setup the 'uniform' variables needed for lighting
// light uniform variables
uniform vec3 ambientColor;
uniform vec3 light1Pos;
uniform vec3 light1Color;
uniform vec3 light2Pos;
uniform vec3 light2Color;
// material properties
uniform vec3 reflectionColor;
uniform float ambientReflectance;
uniform float diffuseReflectance;
uniform float specularReflectance;
uniform float specularExponent;
// attenuation uniforms
uniform float attenuationC0;
uniform float attenuationC1;
uniform float attenuationC2;

// TODO exercise 8.4 add the 'in' variables to receive the interpolated Position and Normal from the vertex shader
in vec3 P_frag;
in vec3 N_frag;

void main()
{
   // TODO exercise 8.4 - phong shading (i.e. Phong reflection model computed in the fragment shader)
   vec3 L1 = normalize(light1Pos - P_frag.xyz);
   vec3 L2 = normalize(light2Pos - P_frag.xyz);
   vec3 V = normalize(camPosition - P_frag.xyz);
   vec3 H1 = normalize(L1 + V);
   vec3 H2 = normalize(L2 + V);
   // ambient component
   vec3 ambient = ambientColor * reflectionColor * ambientReflectance;
   vec4 color = vec4(ambient, 1);

      // TODO exercuse 8.6 - attenuation - light 1
   float distanceLight1 = length(light1Pos - P_frag);
   float att1 = min(1, 1.0/(attenuationC0 + attenuationC1*distanceLight1 + attenuationC2 * distanceLight1 * distanceLight1));

   // TODO exercuse 8.6 - attenuation - light 2
   float distanceLight2 = length(light2Pos - P_frag);
   float att2 = min(1, 1.0/(attenuationC0 + attenuationC1*distanceLight2 + attenuationC2 * distanceLight2 * distanceLight2));

   // diffuse component for light 1
   vec3 diffuse = light1Color * reflectionColor * diffuseReflectance * max(dot(N_frag, L1), 0);

   // specular component for light 1
   vec3 specular = light1Color * specularReflectance * max(pow(dot(N_frag, H1), specularExponent), 0);

   color.xyz += (diffuse + specular) * att1;
   // TODO exercise 8.5 - multiple lights, compute diffuse and specular of light 2
   diffuse = light2Color * reflectionColor * diffuseReflectance * max(dot(N_frag, L2), 0);
   specular = light2Color * specularReflectance * max(pow(dot(N_frag, H2), specularExponent), 0);

   color.xyz += (diffuse + specular) * att2;

   // TODO compute the final shaded color (e.g. add contribution of the (attenuated) lights 1 and 2)


   // TODO set the output color to the shaded color that you have computed
   FragColor = color;
}
// you might have noticed that the shading contribution of multiple lights can fit a for loop nicely
// we will be doing that later on