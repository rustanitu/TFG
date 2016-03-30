#version 400

layout(location = 0) in vec3 VerPos;
layout(location = 1) in vec3 VerClr;

out vec3 EntryPoint;
out vec4 ExitPointCoord;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

void main(void)
{
  EntryPoint = VerClr;
	gl_Position = (ProjectionMatrix * ViewMatrix * ModelMatrix) * vec4(VerPos,1.0);
  ExitPointCoord = gl_Position;  
}
//http://content.gpwiki.org/index.php/OpenGL:Tutorials:3D_Textures
//http://prideout.net/blog/?tag=volume-rendering

//out vec2 texcoords;
//texcoords=normalize(vertex_data.xy)