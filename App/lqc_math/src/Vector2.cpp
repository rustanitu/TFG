#include <math/Vector2.h>

namespace lqc
{
  template<typename T>
  Vector2<T>::Vector2 ()
    : x (T (0)), y (T (0))
  {
  }

  template<typename T>
  Vector2<T>::Vector2 (T value)
    : x(value), y(value)
  {
  }

  template<typename T>
  Vector2<T>::Vector2 (T X, T Y)
    : x (X), y (Y)
  {
  }

  template<typename T>
  double Vector2<T>::Magnitude ()
  {
    return sqrt (x*x + y*y);
  }

  template<typename T>
  void Vector2<T>::Normalize ()
  {
    double a_norm = Magnitude ();
    x = (T)((double)x / a_norm);
    y = (T)((double)y / a_norm);
  }

  template class Vector2<int>;
  template class Vector2<float>;
  template class Vector2<double>;
}