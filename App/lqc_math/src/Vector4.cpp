#include <math/Vector4.h>
#include <cmath>

namespace lqc
{
  template<typename T>
  Vector4<T>::Vector4 ()
    : x (T (0)), y (T (0)), z (T (0)), w (T (0))
  {
  }

  template<typename T>
  Vector4<T>::Vector4 (T value)
    : x (value), y (value), z (value), w (value)
  {
  }

  template<typename T>
  Vector4<T>::Vector4 (T X, T Y, T Z, T W)
    : x (X), y (Y), z (Z), w (W)
  {
  }
  
  template<typename T>
  double Vector4<T>::Magnitude ()
  {
    return sqrt (x*x + y*y + z*z + w*w);
  }
  
  template<typename T>
  void Vector4<T>::Normalize ()
  {
    if (!(x != 0.0f || y != 0.0f || z != 0.0 || w != 0.0))
      return;

    double a_norm = Magnitude ();
    x = (T)((double)x / a_norm);
    y = (T)((double)y / a_norm);
    z = (T)((double)z / a_norm);
    w = (T)((double)w / a_norm); 
  }

  template<typename T>
  bool Vector4<T>::IsZero ()
  {
    return x == (T)0 && y == (T)0 && z == (T)0 && w == (T)0;
  }

  template<typename T>
  void Vector4<T>::Sqrt ()
  {
    x = (T)sqrt (x);
    y = (T)sqrt (y);
    z = (T)sqrt (z);
    w = (T)sqrt (w);
  }
  
  template<typename T>
  bool Vector4<T>::IsEqual (const Vector4<T> v1, const Vector4<T> v2)
  {
    return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w;
  }

  template<typename T>
  Vector4<T> Vector4<T>::Pow (Vector4<T> v, int exponent)
  {
    return Vector4<T> ((T)pow (v.x, exponent), (T)pow (v.y, exponent), (T)pow (v.z, exponent), (T)pow (v.w, exponent));
  }
  
  template<typename T>
  Vector4<T> Vector4<T>::Zero ()
  {
    return Vector4<T> ((T)0);
  }
  
  template<typename T>
  Vector4<T> Vector4<T>::One ()
  {
    return Vector4<T> ((T)1);
  }
  
  template<typename T>
  Vector4<T> Vector4<T>::Multiply (Vector4<T> d, T f)
  {
    return d*f;
  }

  template<typename T>
  Vector4<T> Vector4<T>::Normalize (Vector4<T> v)
  {
    Vector4<T> ret = v;
    ret.Normalize ();
    return ret;
  }

  template class Vector4<int>;
  template class Vector4<float>;
  template class Vector4<double>;
}