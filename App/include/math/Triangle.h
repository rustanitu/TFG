#ifndef LQC_MATH_TRIANGLE_H
#define LQC_MATH_TRIANGLE_H

#include <math/Vector2.h>
#include <math/Vector3.h>

namespace lqc
{
  typedef struct Triangle2D
  {
    Triangle2D ()
      : A(Vector2f(0.0f,0.0f)), B(Vector2f(0.0f,0.0f)), C(Vector2f(0.0f,0.0f))
    {}

    Triangle2D (Vector2f a, Vector2f b, Vector2f c)
      : A(a), B(b), C(c)
    {}

    Vector2f A, B, C; //p1 p2 p3
  } Triangle2D; 

  typedef struct Triangle3D
  {
    Vector3f A, B, C; //p1 p2 p3
  } Triangle3D; 

}

#endif