#ifndef LQC_MATH_POINT2_H
#define LQC_MATH_POINT2_H

namespace lqc
{
  template<typename T>
  class Point2
  {
  public:
    Point2 ();
    Point2 (T value);
    Point2 (T X, T Y);

    T x, y;
  };
}

#endif