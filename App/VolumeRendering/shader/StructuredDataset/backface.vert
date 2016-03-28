#version 430 compatibility

layout(location = 0) in vec3 VerPos;
layout(location = 1) in vec3 VerClr;

out vec3 Color;
//out vec4 ExitPointCoord;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

void main(void)
{
	Color = VerClr;
	gl_Position = (ProjectionMatrix * ViewMatrix * ModelMatrix) * vec4(VerPos, 1.0);
  //ExitPointCoord = gl_Position;  
}