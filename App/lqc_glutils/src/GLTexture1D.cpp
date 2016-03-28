#include <glutils/GLTexture1D.h>

#include <lqc/Utils/Utils.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

namespace gl
{
  GLTexture1D::GLTexture1D (unsigned int length)
  {
    glGenTextures( 1, &m_textureID );
    glBindTexture( GL_TEXTURE_2D, m_textureID );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    m_length = length;
  }

  GLTexture1D::~GLTexture1D ()
  {
    lqc::DebugPrint ("lqc: GLTexture1D\n  - Deleting GLTexture1D\n");
    glDeleteTextures(1, &m_textureID);
    lqc::DebugPrint("lqc: GLTexture1D\n  - GLTexture1D Deleted\n");
  }

  void GLTexture1D::GenerateTexture (GLint min_filter_param, GLint max_filter_param
    , GLint wrap_s_param)
  {
    if (m_textureID != -1)
      DestroyTexture();
    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_1D, m_textureID);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, min_filter_param);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, max_filter_param);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, wrap_s_param);
  }

  void GLTexture1D::SetData (GLvoid* data, GLint internalformat, GLenum format, GLenum type)
  {
    if (m_textureID == -1)
    {
      #if _DEBUG
      printf("lqc: Texture1D not generated yet!\n", m_textureID);
      #endif
      return;
    }
    glTexImage1D (GL_TEXTURE_1D, 0, internalformat, m_length, 0, format, type, data);
    #if _DEBUG
    printf("lqc: Texture1D generated with id %d!\n", m_textureID);
    #endif
  }

  GLuint GLTexture1D::GetTextureID ()
  {
    return m_textureID;
  }

  unsigned int GLTexture1D::GetLength ()
  {
    return m_length;
  }

  void GLTexture1D::DestroyTexture ()
  {
    GLint temp_texture = m_textureID;
    glDeleteTextures(1, &m_textureID);
    #if _DEBUG
    printf("lqc: Texture1D with id %d destroyed!\n", temp_texture);
    #endif
    m_textureID = -1;
  }
}