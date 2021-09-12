#version 330 core

in float age;
out vec4 fragColor;

// TODO 2.6: should receive the age of the particle as an input variable

void main()
{
    float maxAge = 10.0;
    // TODO 2.4 set the alpha value to 0.2 (alpha is the 4th value of the output color)
    float alpha = 0.2;

    // TODO 2.5 and 2.6: improve the particles appearance
    vec2 center = vec2(0.5, 0.5);
    float dist = distance(center, gl_PointCoord);
    alpha = mix(1.0, 0.0, dist*2);

    alpha = mix(alpha, 0.0, age/maxAge);

    vec3 startColor = vec3(1.0, 1.0, 0.05);
    vec3 midColor = vec3(1.0, 0.5, 0.01);
    vec3 endColor = vec3(0.0);

    float midPoint = maxAge / 2.0;
    vec3 color;
    if(age <= midPoint)
      color = mix(startColor, midColor, age/midPoint);
    else
      color = mix(midColor, endColor, (age-midPoint)/midPoint);

    // remember to replace the default output (vec4(1.0,1.0,1.0,1.0)) with the color and alpha values that you have computed
    fragColor = vec4(color, alpha);

}