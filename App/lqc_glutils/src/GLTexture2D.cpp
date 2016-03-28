#include <glutils/GLTexture2D.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

namespace gl
{
  GLTexture2D::GLTexture2D (unsigned int width, unsigned int height)
  {
    glGenTextures( 1, &m_textureID );
    glBindTexture( GL_TEXTURE_2D, m_textureID );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    m_width = width;
    m_height = height;
  }

  GLTexture2D::~GLTexture2D ()
  {
    glDeleteTextures(1, &m_textureID);
  }

  void GLTexture2D::GenerateTexture (GLint min_filter_param, GLint max_filter_param
    , GLint wrap_s_param, GLint wrap_t_param)
  {
    if (m_textureID != -1)
      DestroyTexture();
    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter_param);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, max_filter_param);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s_param);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t_param);
  }

  void GLTexture2D::SetData (GLvoid* data, GLint internalformat, GLenum format, GLenum type)
  {
    if (m_textureID == -1)
    {
      printf("lqc: Texture2D not generated yet!\n", m_textureID);
      return;
    }
    //para texturas grandes GL_PROXY_TEXTURE_2D
    glTexImage2D(GL_TEXTURE_2D, 0, internalformat, m_width, m_height, 0, format, type, data);
    printf("lqc: Texture2D generated with id %d!\n", m_textureID);
  }

  GLuint GLTexture2D::GetTextureID ()
  {
    return m_textureID;
  }


  unsigned int GLTexture2D::GetWidth ()
  {
    return m_width;
  }

  unsigned int GLTexture2D::GetHeight ()
  {
    return m_height;
  }

  void GLTexture2D::DestroyTexture ()
  {
    GLint temp_texture = m_textureID;
    glDeleteTextures(1, &m_textureID);
    printf("lqc: Texture2D with id %d destroyed!\n", temp_texture);
    m_textureID = -1;
  }
}