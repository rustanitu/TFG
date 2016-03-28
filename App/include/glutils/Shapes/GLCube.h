#ifndef LQC_GL_CUBE_H
#define LQC_GL_CUBE_H

#include <lqc/Utils/Utils.h>
#include <glutils/GLUtils.h>

namespace gl
{
  class GLCube
  {
  public:
    GLCube();
    ~GLCube();
  
    void create();
    void destroy();
    void draw(void);
  private:
    GLuint BufferIds[3];
  };

}

#endif