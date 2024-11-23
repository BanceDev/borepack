#vertex
#version 460 core
layout (location = 0) in vec3 VertPosition;
layout (location = 1) in vec2 VertTexCoord;
layout (location = 2) in vec2 VertLightmap;

layout (location = 0) uniform mat4 ProjectionMatrix;
layout (location = 1) uniform mat4 ViewMatrix;
layout (location = 2) uniform mat4 ModelMatrix;

out vec2 UV;
out vec2 LightmapUV;

void main()
{
    mat4 ModelViewProjectionMatrix = ProjectionMatrix * ViewMatrix * ModelMatrix;
    gl_Position = ModelViewProjectionMatrix * vec4(VertPosition, 1.0);
    UV = VertTexCoord;
    LightmapUV = VertLightmap;
}

#fragment
#version 460 core
out vec4 FragColor;

in vec2 UV;
in vec2 LightmapUV;

uniform sampler2D Texture0;
uniform sampler2D Texture1;

// Define the number of quantization levels (e.g., 5 levels)
const float NumLevels = 20.0;

void main()
{
    vec4 BaseColor = texture(Texture0, UV);
    vec4 LightColor = vec4(texture(Texture1, LightmapUV).rrr, 1.0);
    
    // Quantize the lightmap color to NumLevels steps
    LightColor.rgb = floor(LightColor.rgb * NumLevels) / (NumLevels - 1.0);

    // Apply lighting
    FragColor = (BaseColor * 2.0) * LightColor;
}

