#version 330 core
out vec4 FragColor;

in VS_OUT {
   vec3 normal;
   vec3 position;
} fin;

uniform vec3 cameraPos;
uniform samplerCube skybox;

uniform float reflectionFactor;
uniform float n2;
const float n1 = 1.0;

void main()
{
    vec3 I = normalize(fin.position - cameraPos); // incidence vector
    vec3 V = -I; // view vector (surface to camera)
    vec3 N = normalize(fin.normal); // surface normal


    // TODO exercise 10.1 - reflect camera to fragment vector and sample the skybox with the reflected direction
    vec3 reflection = reflect(I, N);
    vec4 reflectColor = texture(skybox, reflection);

    // TODO exercise 10.2 - refract the camera to fragment vector and sample the skybox with the reffracted direction
    vec3 refraction = refract(I, N, n1/n2);
    vec4 refractColor = texture(skybox, refraction);

    // TODO exercise 10.3 - implement the Schlick approximation of the Fresnel factor and set "reflectionProportion" accordingly
    float R0 = pow((n1-n2)/(n1+n2), 2);
    float angle = dot(N, V); //No need to divide by length as they are normalized
    float reflectionProportion = R0 + (1.0f-R0) * pow(1.0f - angle, 5);
    //float reflectionProportion = reflectionFactor;


    // we combine reflected and refracted color here
    FragColor = reflectionProportion * reflectColor + (1.0 - reflectionProportion) * refractColor;
}