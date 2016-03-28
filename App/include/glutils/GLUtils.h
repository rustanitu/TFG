#ifndef LQC_GL_UTILS_H
#define LQC_GL_UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <ctime>
#include <vector>
#include <string>
#include <GL/glew.h>
#include <GL/freeglut.h>

#define LQC_GL_UTILS_LIB

namespace gl
{
  typedef struct Vertex
  {
	  float Position[4];
	  float Color[4];
  } Vertex;

  void ExitOnGLError (const char* error_message);
  void ShaderInfoLog (GLuint obj);
  void ProgramInfoLog (GLuint obj);
  int  OglError(char *file, int line);

  GLuint LoadShader (const char* file_name, GLenum shader_type);
  char* TextFileRead (const char* file_name);
}

#endif