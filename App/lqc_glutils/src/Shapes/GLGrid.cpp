#include <glutils/Shapes/GLGrid.h>

namespace gl
{
  Grid_21::Grid_21()
  {
  }

  Grid_21::~Grid_21()
  {
    destroy();
  }

  void Grid_21::create()
  {
  }

  void Grid_21::destroy()
  {
    glDeleteBuffers(2, &BufferIds[1]);
	  glDeleteVertexArrays(1, &BufferIds[0]);
	  ExitOnGLError("ERROR: Could not destroy the buffer objects");	
  }

  void Grid_21::draw(void)
  {
  }

}