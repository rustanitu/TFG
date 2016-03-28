#ifndef LQC_MATH_TETRAHEDRON_H
#define LQC_MATH_TETRAHEDRON_H

#include <math/Vector3.h>

namespace lqc
{
  typedef struct Tetrahedron
  {
    Vector3f A, B, C, D; //p1 p2 p3
  } Tetrahedron;
}

#endif