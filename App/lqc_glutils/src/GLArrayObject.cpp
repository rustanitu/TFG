#include <glutils/GLArrayObject.h>

namespace gl
{
  void GLArrayObject::Unbind ()
  {
    glBindVertexArray (0);
  }

  GLArrayObject::GLArrayObject (unsigned int number_of_vertex_attribute_locations)
    : m_arrays (0), m_id (0)
  {
    glGenVertexArrays (1, &m_id);
    gl::ExitOnGLError ("ERROR: Could not generate the VAO");

    Bind ();
    EnableGenericArraysAttribs (number_of_vertex_attribute_locations);
    Unbind ();
  }

  GLArrayObject::~GLArrayObject ()
  {
    glDeleteVertexArrays (1, &m_id);
    gl::ExitOnGLError ("ERROR: Could not destroy the vertex array");
  }

  void GLArrayObject::Bind ()
  {
    GLint currentvaoid;
    glGetIntegerv (GL_VERTEX_ARRAY_BINDING, &currentvaoid);
    if (currentvaoid != m_id)
    {
      glBindVertexArray (m_id);
      gl::ExitOnGLError ("ERROR: Could not bind the VAO");
    }
  }

  void GLArrayObject::DrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
  {
    Bind ();
    glDrawElements (mode, count, type, indices);
    gl::ExitOnGLError ("lqc: error in GLVAO::DrawElements(...)");
  }

  void GLArrayObject::SetVertexAttribPointer (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer)
  {
    if (index < m_arrays)
      glVertexAttribPointer (index, size, type, normalized, stride, pointer);
    gl::ExitOnGLError ("ERROR: Could not set VAO attributes");

  }

  void GLArrayObject::EnableGenericArraysAttribs (unsigned int number_of_arrays)
  {
    for (m_arrays = 0; m_arrays < number_of_arrays && m_arrays < GL_MAX_VERTEX_ATTRIBS; m_arrays++)
      glEnableVertexAttribArray (m_arrays);
    gl::ExitOnGLError ("ERROR: Could not enable vertex attributes");
  }

  void GLArrayObject::DisableGenericArraysAttribs ()
  {
    for (int i = 0; i < (int)m_arrays; i++)
      glDisableVertexAttribArray (i);
  }

  GLuint GLArrayObject::GetID ()
  {
    return m_id;
  }
}