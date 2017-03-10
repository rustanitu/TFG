#version 430 compatibility

in vec3 Color;
//in vec4 ExitPointCoord;

layout (location = 0) out vec4 FragColor;

void main(void)
{
  FragColor = vec4(Color, 1.0);
}