#ifndef LQC_GL_TEXTURE3D_H
#define LQC_GL_TEXTURE3D_H

#include <glutils/GLUtils.h>
#include <glutils/GLTexture.h>

namespace gl
{
  class GLTexture3D : public GLTexture
  {
  public:
    GLTexture3D (unsigned int width, unsigned int height, unsigned int depth);
    ~GLTexture3D ();

    void GenerateTexture (GLint min_filter_param, GLint max_filter_param
    , GLint wrap_s_param, GLint wrap_t_param, GLint wrap_r_param);

    void SetData (GLvoid* data, GLint internalformat, GLenum format, GLenum type);

    GLuint GetTextureID ();

    unsigned int GetWidth ();
    unsigned int GetHeight ();
    unsigned int GetDepth ();
  private:
    void DestroyTexture ();
    unsigned int m_width;
    unsigned int m_height;
    unsigned int m_depth;
    GLuint m_textureID;
  };
}

#endif