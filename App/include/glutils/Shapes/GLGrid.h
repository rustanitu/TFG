#ifndef LQC_GL_GRID_H
#define LQC_GL_GRID_H

#include <glutils/GLUtils.h>

namespace gl
{
  class Grid_21
  {
  public:
    Grid_21();
    ~Grid_21();
  
    void create();
    void destroy();
    void draw(void);
  private:
    GLuint BufferIds[3];
  };

}

#endif