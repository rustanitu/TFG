#include <glutils/GLFrameBufferObject.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

namespace gl
{
  void GLFrameBufferObject::Unbind ()
  {
    glBindFramebuffer (GL_FRAMEBUFFER, 0);
  }

  GLFrameBufferObject::GLFrameBufferObject ()
  {
    glGenFramebuffers (1, &m_id);
  }
  
  GLFrameBufferObject::~GLFrameBufferObject ()
  {
    glDeleteFramebuffers (1, &m_id);
  }

  void GLFrameBufferObject::Bind ()
  {
    glBindFramebuffer (GL_FRAMEBUFFER, m_id);
  }

  GLint GLFrameBufferObject::GetMaxLayers ()
  {
    GLint res;
    glGetIntegerv (GL_MAX_FRAMEBUFFER_LAYERS, &res);
    return res;
  }

  GLint GLFrameBufferObject::GetMaxSamples ()
  {
    GLint res;
    glGetIntegerv (GL_MAX_FRAMEBUFFER_SAMPLES, &res);
    return res;
  }

  GLint GLFrameBufferObject::GetFramebufferWidth ()
  {
    GLint res;
    glGetIntegerv (GL_MAX_FRAMEBUFFER_WIDTH, &res);
    return res;
  }

  GLint GLFrameBufferObject::GetFramebufferHeight ()
  {
    GLint res;
    glGetIntegerv (GL_MAX_FRAMEBUFFER_HEIGHT, &res);
    return res;
  }

  GLint GLFrameBufferObject::GetMaxColorAttachments ()
  {
    GLint maxcolorattachments;
    glGetIntegerv (GL_MAX_COLOR_ATTACHMENTS, &maxcolorattachments);
    return maxcolorattachments;
  }
}