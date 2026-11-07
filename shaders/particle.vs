#version 330 core

layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec3 aColor;
layout (location = 2) in float aAlpha;

out vec3 ParticleColor;
out float ParticleAlpha;

void main()
{
    gl_Position = vec4(aPosition, 0.0, 1.0);

    gl_PointSize = 5.0;

    ParticleColor = aColor;
    ParticleAlpha = aAlpha;
}
