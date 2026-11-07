#version 330 core

in vec3 ParticleColor;
in float ParticleAlpha;

out vec4 FragColor;

void main()
{
    vec2 centered = gl_PointCoord - vec2(0.5);

    float distanceFromCenter = length(centered);

    if (distanceFromCenter > 0.5)
        discard;

    float glow = 1.0 - smoothstep(0.2, 0.5, distanceFromCenter);

    FragColor = vec4(
        ParticleColor * glow,
        ParticleAlpha * glow
    );
}
