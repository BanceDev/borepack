#vertex
#version 460 core
layout (location = 0) in vec3 VertPosition;
layout (location = 1) in vec2 VertTexCoord;

layout (location = 0) uniform mat4 ProjectionMatrix;
layout (location = 1) uniform mat4 ViewMatrix;
layout (location = 2) uniform mat4 ModelMatrix;

out vec2 UV;

void main()
{
    mat4 ModelViewProjectionMatrix = ProjectionMatrix * ViewMatrix * ModelMatrix;
    gl_Position = ModelViewProjectionMatrix * vec4(VertPosition, 1.0);
    UV = VertTexCoord;
}

#fragment
#version 460 core
out vec4 FragColor;

in vec2 UV;

uniform sampler2D Texture0;
uniform float Time;

void main()
{
    // Apply sine wave distortion to UV coordinates for water ripple effect
    float wave = sin(UV.x * 10.0 + Time * 2.0) * 0.02;
    float wave2 = cos(UV.y * 10.0 + Time * 2.5) * 0.02;
    vec2 DistortedUV = UV + vec2(wave, wave2);

    // Sample the texture with distorted UVs
    vec4 BaseColor = texture(Texture0, DistortedUV);
    FragColor = BaseColor;
}

