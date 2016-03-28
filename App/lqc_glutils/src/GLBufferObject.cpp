#include <glutils/GLBufferObject.h>

namespace gl
{
  void GLBufferObject::Unbind (GLenum target)
  {
    glBindBuffer (target, 0);
  }

  GLBufferObject::GLBufferObject (GLenum target)
    : m_id (0), m_target (target)
  {
    glGenBuffers (1, &m_id);
    gl::ExitOnGLError ("ERROR: Could not generate the Buffer Object");
  }

  GLBufferObject::~GLBufferObject ()
  {
    glDeleteBuffers (1, &m_id);
    gl::ExitOnGLError ("ERROR: Could not destroy the buffer object");
  }

  void GLBufferObject::Bind ()
  {
    glBindBuffer (m_target, m_id);
    gl::ExitOnGLError ("ERROR: Could not bind the Buffer Object");
  }

  void GLBufferObject::Unbind ()
  {
    glBindBuffer (m_target, 0);
  }

  void GLBufferObject::SetBufferData (GLsizeiptr size, const GLvoid *data, GLenum usage)
  {
    Bind ();
    glBufferData (m_target, size, data, usage);
    gl::ExitOnGLError ("ERROR: Could not set Buffer Object data");
  }

  GLuint GLBufferObject::GetID ()
  {
    return m_id;
  }
}