#ifndef LQC_GL_STACKMATRIX_H
#define LQC_GL_STACKMATRIX_H

#include <glm/glm.hpp>
#include <stack>

namespace gl
{
  class StackMatrix
  {
  public:
    StackMatrix ();
    ~StackMatrix ();
    
    enum STACK
    {
      MODELVIEW,
      PROJECTION,
      TEXTURE,
    };

    void MatrixMode (STACK mode);
   
    void LoadIdentity ();
    void MultiplyMatrix (glm::mat4 m, bool rightmult = false);
    void SetCurrentMatrix (glm::mat4 m);
    void PopMatrix ();
    void PushMatrix ();
    void ResetStack (); 

    void PushIdentity (); 
    glm::mat4 GetMatrix ();
    glm::mat4 PopAndGetMatrix ();

  private:
    std::stack<glm::mat4>* GetCurrentStack ();
    STACK m_currentmode;
    std::stack<glm::mat4> m_stk_modelview;
    std::stack<glm::mat4> m_stk_projection;
    std::stack<glm::mat4> m_stk_texture;
  };
}

#endif