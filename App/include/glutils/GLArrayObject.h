#ifndef LQC_GL_ARRAY_OBJECT_H
#define LQC_GL_ARRAY_OBJECT_H

#include <glutils/GLUtils.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

namespace gl
{
  class GLArrayObject
  {
  public:
    static void Unbind ();

    GLArrayObject (unsigned int number_of_vertex_attribute_locations = 0);
    ~GLArrayObject ();

    void Bind ();

    void DrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices = (GLvoid*)0);

    void SetVertexAttribPointer (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer);

    void EnableGenericArraysAttribs (unsigned int number_of_arrays);
    void DisableGenericArraysAttribs ();

    GLuint GetID ();

  private:
    GLuint m_id;
    unsigned int m_arrays;
  };
}

#endif