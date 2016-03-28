#include <glutils/GLStackMatrix.h>

#include <math/MUtils.h>
#include <math/Matrix4.h>

namespace gl
{
  StackMatrix::StackMatrix ()
  {
    m_currentmode = MODELVIEW;
    m_stk_modelview.push(glm::mat4 ());
    m_stk_projection.push (glm::mat4 ());
    m_stk_texture.push (glm::mat4 ());
  }

  StackMatrix::~StackMatrix ()
  {}

  void StackMatrix::MatrixMode (STACK mode)
  {
    m_currentmode = mode;
  }

  void StackMatrix::LoadIdentity ()
  {
    std::stack<glm::mat4>* current_stack = GetCurrentStack ();
    if (!current_stack)
    {
      printf("lqc: current stack not found\n");
      return;
    }
    current_stack->top () = glm::mat4 ();
  }

  void StackMatrix::MultiplyMatrix (glm::mat4 m, bool rightmult)
  {
    std::stack<glm::mat4>* current_stack = GetCurrentStack ();
    if (!current_stack)
    {
      printf("lqc: Current stack not found\n");
      return;
    }
    glm::mat4 stackm = current_stack->top ();
    //é na ordem contrária da aplicação a multiplicação das matrizes
    if (rightmult)
      current_stack->top () = stackm * m;
    else
      current_stack->top() = m * stackm;
  }

  void StackMatrix::SetCurrentMatrix (glm::mat4 m)
  {
    GetCurrentStack ()->top () = m;
  }

  void StackMatrix::PopMatrix ()
  {
    std::stack<glm::mat4>* current_stack = GetCurrentStack ();
    if (!current_stack)
    {
      printf("lqc: Current stack not found\n");
      return;
    }
    current_stack->pop();
  }

  void StackMatrix::PushMatrix ()
  {
    std::stack<glm::mat4>* current_stack = GetCurrentStack ();
    if (!current_stack)
    {
      printf("lqc: Current stack not found\n");
      return;
    }
    glm::mat4 top = current_stack->top ();
    current_stack->push(top);
  }

  void StackMatrix::ResetStack ()
  {
    switch (m_currentmode)
    {
    case MODELVIEW:
      m_stk_modelview = std::stack<glm::mat4> ();
      break;
    case PROJECTION:
      m_stk_projection = std::stack<glm::mat4> ();
      break;
    case TEXTURE:
      m_stk_texture = std::stack<glm::mat4> ();
      break;
    default:
      printf("lqc: Matrix_Mode Unknown at GLStackMatrix.ResetMatrix()\n");
      break;
    }
  }

  void StackMatrix::PushIdentity ()
  {
    std::stack<glm::mat4>* current_stack = GetCurrentStack ();
    if (!current_stack)
    {
      printf("lqc: Current stack not found\n");
      return;
    }
    current_stack->push (glm::mat4 ());
  }

  glm::mat4 StackMatrix::GetMatrix ()
  {
    std::stack<glm::mat4>* current_stack = GetCurrentStack ();
    if (!current_stack)
    {
      printf("lqc: Current stack not found, returning IDENTITY_MATRIX\n");
      return glm::mat4 ();
    }
    glm::mat4 ret = current_stack->top ();
    return ret;
  }

  glm::mat4 StackMatrix::PopAndGetMatrix ()
  {
    std::stack<glm::mat4>* current_stack = GetCurrentStack ();
    if (!current_stack)
    {
      printf("lqc: Current stack not found, returning IDENTITY_MATRIX\n");
      return glm::mat4 ();
    }
    glm::mat4 ret = current_stack->top ();
    current_stack->pop();
    return ret;
  }

  std::stack<glm::mat4>* StackMatrix::GetCurrentStack ()
  {
    switch (m_currentmode)
    {
    case MODELVIEW:
      return &m_stk_modelview;
    case PROJECTION:
      return &m_stk_projection;
    case TEXTURE:
      return &m_stk_texture;
    default:
      printf("lqc: Current Matrix_Mode Unknown at GLStackMatrix.GetCurrentStack() function\n");
      return NULL;
    }
  }
}