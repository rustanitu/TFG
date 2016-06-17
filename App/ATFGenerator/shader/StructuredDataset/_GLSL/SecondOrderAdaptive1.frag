#version 330

in vec4 ex_Color;
layout (location = 0) out vec4 colorOut;

void main(void)
{
  colorOut = ex_Color;
}