#ifndef LQC_GL_TEXTURE1D_H
#define LQC_GL_TEXTURE1D_H

#include <glutils/GLUtils.h>
#include <glutils/GLTexture.h>

namespace gl
{
  class GLTexture1D : public GLTexture
  {
  public:
    GLTexture1D (unsigned int length);
    ~GLTexture1D ();

    void GenerateTexture (GLint min_filter_param, GLint max_filter_param
    , GLint wrap_s_param);

    void SetData (GLvoid* data, GLint internalformat, GLenum format, GLenum type);
    
    GLuint GetTextureID ();

    unsigned int GetLength ();
  private:
    void DestroyTexture ();
    unsigned int m_size;
    unsigned int m_length;
    GLuint m_textureID;
  };
}

#endif