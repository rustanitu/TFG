#include <math/Vector3.h>

namespace lqc
{
  template<typename T>
  Vector3<T>::Vector3 ()
    : x (T (0)), y (T (0)), z (T (0))
  {
  }

  template<typename T>
  Vector3<T>::Vector3 (T value)
    : x (value), y (value), z (value)
  {
  }

  template<typename T>
  Vector3<T>::Vector3 (T X, T Y, T Z)
    : x (X), y (Y), z (Z)
  {
  }

  template<typename T>
  double Vector3<T>::Magnitude ()
  {
    return sqrt (x*x + y*y + z*z);
  }

  template<typename T>
  void Vector3<T>::Normalize ()
  {
     double a_norm = Magnitude ();
     if (a_norm > 0.000001)
     {
       x = (T)((double)x / a_norm);
       y = (T)((double)y / a_norm);
       z = (T)((double)z / a_norm);
     }
   }

  template<typename T>
  void Vector3<T>::normalize ()
  {
    Normalize ();
  }

  template<typename T>
  bool Vector3<T>::IsZero ()
  {
    return x == (T)0 && y == (T)0 && z == (T)0;
  }

  template<typename T>
  void Vector3<T>::Sqrt ()
  {
    x = (T)sqrt (x);
    y = (T)sqrt (y);
    z = (T)sqrt (z);
  }

  template<typename T>
  void Vector3<T>::Set (T X, T Y, T Z)
  {
    x = (T)X;
    y = (T)Y;
    z = (T)Z;
  }

  template<typename T>
  bool Vector3<T>::IsEqual (const Vector3<T> v1, const Vector3<T> v2)
  {
    return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
  }

  template<typename T>
  Vector3<T> Vector3<T>::Pow (Vector3<T> v, int exponent)
  {
    return Vector3<T> ((T)pow (v.x, exponent), (T)pow (v.y, exponent), (T)pow (v.z, exponent));
  }

  template<typename T>
  Vector3<T> Vector3<T>::Zero ()
  {
    return Vector3<T> ((T)0, (T)0, (T)0);
  }

  template<typename T>
  Vector3<T> Vector3<T>::One ()
  {
    return Vector3<T> ((T)1, (T)1, (T)1);
  }

  template<typename T>
  Vector3<T> Vector3<T>::Multiply (Vector3<T> d, T f)
  {
    return d*f;
  }

  template<typename T>
  Vector3<T> Vector3<T>::Normalize (Vector3<T> v)
  {
    Vector3<T> ret = v;
    ret.Normalize ();
    return ret;
  }

  template class Vector3<int>;
  template class Vector3<float>;
  template class Vector3<double>;
}