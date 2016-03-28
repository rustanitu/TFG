#ifndef LQC_GL_FRAMEBUFFER_OBJECT_H
#define LQC_GL_FRAMEBUFFER_OBJECT_H

#include <GL/glew.h>
#include <GL/freeglut.h>

namespace gl
{
  class GLFrameBufferObject
  {
  public:
    static void Unbind ();
    
    GLFrameBufferObject ();
    ~GLFrameBufferObject ();

    void Bind ();

    GLint GetMaxLayers ();
    GLint GetMaxSamples ();
    GLint GetFramebufferWidth ();
    GLint GetFramebufferHeight ();
    GLint GetMaxColorAttachments ();

  private:
    GLuint m_id;
  };
}

#endif