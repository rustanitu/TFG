#ifndef LQC_MATH_QUADRATICINTERPOLATION_H
#define LQC_MATH_QUADRATICINTERPOLATION_H

#include <iostream>
#include <vector>
#include <cassert>

namespace lqc
{
  //http://isezen.com/2012/01/15/quadratic-interpolation-three-point/
  class QuadraticIntepolation
  {
    QuadraticIntepolation(float x0, float y0, float x1, float y1, float x2, float y2)
    {
      float a0 = y0 / ((x0 - x1)*(x0 - x2));
      float a1 = y1 / ((x1 - x0)*(x1 - x2));
      float a2 = y2 / ((x2 - x0)*(x2 - x1));

      A = a0 + a1 + a2;
      B = -(a0*(x1 + x2) + a1*(x0 + x2) + a2*(x0 + x1));
      C = a0*x1*x2 + a1*x0*x2 + a2*x0*x1;
    }

    float Get(float t)
    {
      assert (("The parameter value must be [0.0, 1.0]", t >= 0.0f && t <= 1.0f));
      return (A*t + B) * t + C;
    }

  private:
    float A, B, C;

  };
}

#endif