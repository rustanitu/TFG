#ifndef LQC_GL_TEXTURE2D_H
#define LQC_GL_TEXTURE2D_H

#include <glutils/GLUtils.h>

namespace gl
{
  class GLTexture2D
  {
  public:
    GLTexture2D (unsigned int width, unsigned int height);
    ~GLTexture2D ();

    void GenerateTexture (GLint min_filter_param, GLint max_filter_param
    , GLint wrap_s_param, GLint wrap_t_param);

    void SetData (GLvoid* data, GLint internalformat, GLenum format, GLenum type);
    
    GLuint GetTextureID ();

    unsigned int GetWidth ();
    unsigned int GetHeight ();
  private:
    void DestroyTexture ();
    unsigned int m_width;
    unsigned int m_height;
    GLuint m_textureID;
  };
}

#endif