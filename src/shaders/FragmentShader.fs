#version 450 core

layout(location = 0) out vec4 fragColor;

void main()
{
    vec3 circleColor = vec3(0.85, 0.35, 0.2);   
    fragColor.rgb = circleColor;
};