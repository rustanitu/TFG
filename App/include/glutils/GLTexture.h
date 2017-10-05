#ifndef LQC_GL_TEXTURE_H
#define LQC_GL_TEXTURE_H

#include <glutils/GLUtils.h>

namespace gl
{
  class GLTexture
  {
  public:
    virtual void SetData(GLvoid* data, GLint internalformat, GLenum format, GLenum type) = 0;
    virtual GLuint GetTextureID() = 0;
  };
}

#endif