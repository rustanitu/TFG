#version 400

in vec3 EntryPoint;
in vec4 ExitPointCoord;

uniform sampler2D TexturePlane;

layout (location = 0) out vec4 FragColor;

void main(void)
{
  FragColor = vec4(texture(TexturePlane, EntryPoint.xy).rgb, 1.0);
}