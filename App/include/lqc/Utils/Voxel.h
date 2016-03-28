#ifndef LQC_UTILS_VOXEL_H
#define LQC_UTILS_VOXEL_H

#include <math/Vector3.h>

/**
*            z
*            /\
*            |
*            |
*            0-----------2
*           /|          /|
*          / |         / |
*         /  |        /  |
*        /   |       /   |
*       1-----------3    |
*       |    |      |    |
*       |    4-----------6-------------->y
*       |   /       |   /
*       |  /        |  /
*       | /         | / 
*       |/          |/
*       5-----------7
*      /
*     /
*    \/
*    x
**/
namespace lqc
{
  class Voxel
  {
  public:
    Voxel ();
    Voxel (Vector3f vmin, Vector3f vmax);
    ~Voxel ();

    float GetScalar (Vector3f pos);

    float m_scalar_values[8];
    Vector3f m_vertex[8];
  private:
  };
}

#endif