#ifndef LQC_MATH_VECTOR3_H
#define LQC_MATH_VECTOR3_H

#include <cmath>
#include <cstdio>
#include <iostream>

#include <math/Matrix.h>

namespace lqc
{
  template<typename T>
  class Vector3
  {
  public:
    Vector3 ();
    Vector3 (T value);
    Vector3 (T X, T Y, T Z);

    double Magnitude ();
    void Normalize ();
    void normalize ();
    bool IsZero ();
    void Sqrt ();
    void Set (T X, T Y, T Z);

    static bool IsEqual (const Vector3<T> v1, const Vector3<T> v2);
    static Vector3<T> Pow (Vector3<T> v, int exponent);
    static Vector3<T> Zero ();
    static Vector3<T> One ();
    static Vector3<T> Multiply (Vector3<T> d, T f);
    static Vector3<T> Normalize (Vector3<T> v);

    //friend std::ostream& operator <<(std::ostream&, const Vector3<T>&);
    //friend std::istream& operator >>(std::istream&, Vector3<T>&);
    //friend Vector3<T> operator + (const T, const Vector3<T>);
    //friend Vector3<T> operator * (const T, const Vector3<T>);

    Vector3<T>& operator =(const Vector3<T>);

    Vector3<T> operator + (const Vector3<T>) const;
    Vector3<T> operator + (const T) const;
    Vector3<T>& operator ++ ();
    Vector3<T>& operator ++ (int);
    Vector3<T>& operator += (const Vector3<T>);

    Vector3<T> operator - (const Vector3<T>) const;
    Vector3<T> operator - (const T) const;
    Vector3<T>& operator -= (const Vector3<T>);
     
    Vector3<T> operator * (const Vector3<T>) const;
    Vector3<T> operator * (const T) const;

    Vector3<T> operator / (const Vector3<T>) const;
    Vector3<T> operator / (const T) const;

    bool operator== (const Vector3<T> v);
    bool operator!= (const Vector3<T> v);

    T x, y, z;
  };

  template<typename T>
  std::ostream& operator << (std::ostream& out, const Vector3<T>& v)
  {
    out << v.x << " " << v.y << " " << v.z;
    return out;
  }

  template<typename T>
  std::istream& operator >> (std::istream& in, Vector3<T>& v)
  {
    in >> v.x >> v.y >> v.z;
    return in;
  }

  template<typename T>
  Vector3<T> operator + (const T t, const Vector3<T> v)
  {
    Vector3<T> temp;
    temp.x = v.x + t;
    temp.y = v.y + t;
    temp.z = v.z + t;
    return temp;
  }

  template<typename T>
  Vector3<T> operator * (const T t, const Vector3<T> v)
  {
    Vector3<T> temp;
    temp.x = v.x * t;
    temp.y = v.y * t;
    temp.z = v.z * t;
    return temp;
  }

  template<typename T>
  Vector3<T> operator / (const T t, const Vector3<T> v)
  {
    Vector3<T> temp;
    temp.x = v.x / t;
    temp.y = v.y / t;
    temp.z = v.z / t;
    return temp;
  }

  template<typename T>
  Vector3<T>& Vector3<T>::operator = (const Vector3<T> v)
  {
    this->x = v.x;
    this->y = v.y;
    this->z = v.z;
    return *this;
  }

  template<typename T>
  Vector3<T> Vector3<T>::operator + (const Vector3<T> v) const
  {
    Vector3<T> temp;
    temp.x = this->x + v.x;
    temp.y = this->y + v.y;
    temp.z = this->z + v.z;
    return temp;
  }

  template<typename T>
  Vector3<T> Vector3<T>::operator + (const T t) const
  {
    Vector3<T> temp;
    temp.x = this->x + t;
    temp.y = this->y + t;
    temp.z = this->z + t;
    return temp;
  }

  template<typename T>
  Vector3<T>& Vector3<T>::operator ++ ()
  {
    ++(this->x);
    ++(this->y);
    ++(this->z);
    return *this;
  }

  template<typename T>
  Vector3<T>& Vector3<T>::operator ++ (int)
  {
    this->x++;
    this->y++;
    this->z++;
    return *this;
  }

  template<typename T>
  Vector3<T>& Vector3<T>::operator += (const Vector3<T> v)
  {
    this->x += v.x;
    this->y += v.y;
    this->z += v.z;
    return *this;
  }

  template<typename T>
  Vector3<T> Vector3<T>::operator - (const Vector3<T> v) const
  {
    Vector3<T> temp;
    temp.x = this->x - v.x;
    temp.y = this->y - v.y;
    temp.z = this->z - v.z;
    return temp;
  }

  template<typename T>
  Vector3<T> Vector3<T>::operator - (const T t) const
  {
    Vector3<T> temp;
    temp.x = this->x - t;
    temp.y = this->y - t;
    temp.z = this->z - t;
    return temp;
  }

  template<typename T>
  Vector3<T>& Vector3<T>::operator -= (const Vector3<T> v)
  {
    this->x -= v.x;
    this->y -= v.y;
    this->z -= v.z;
    return *this;
  }

  template<typename T>
  Vector3<T> Vector3<T>::operator * (const Vector3<T> v) const
  {
    Vector3<T> temp;
    temp.x = this->x * v.x;
    temp.y = this->y * v.y;
    temp.z = this->z * v.z;
    return temp;
  }

  template<typename T>
  Vector3<T> Vector3<T>::operator * (const T t) const
  {
    Vector3<T> temp;
    temp.x = this->x * t;
    temp.y = this->y * t;
    temp.z = this->z * t;
    return temp;
  }

  template<typename T>
  Vector3<T> Vector3<T>::operator / (const Vector3<T> v) const
  {
    Vector3<T> temp;
    temp.x = this->x / v.x;
    temp.y = this->y / v.y;
    temp.z = this->z / v.z;
    return temp;
  }

  template<typename T>
  Vector3<T> Vector3<T>::operator / (const T t) const
  {
    Vector3<T> temp;
    temp.x = this->x / t;
    temp.y = this->y / t;
    temp.z = this->z / t;
    return temp;
  }

  template<typename T>
  bool Vector3<T>::operator == (const Vector3<T> v)
  {
    return (this->x == v.x) && (this->y == v.y) && (this->z == v.z);
  }

  template<typename T>
  bool Vector3<T>::operator != (const Vector3<T> v)
  {
    return !((this->x == v.x) && (this->y == v.y) && (this->z == v.z));
  }

  template<typename T>
  Vector3<T> operator * (const Mat<T, 3, 3> m, const Vector3<T> v)
  {
    Vector3<T> vret;
    vret.x = v.x*m.m[0] + v.y*m.m[1] + v.z*m.m[2];
    vret.y = v.x*m.m[3] + v.y*m.m[4] + v.z*m.m[5];
    vret.z = v.x*m.m[6] + v.y*m.m[7] + v.z*m.m[8];

    return vret;
  }

  typedef Vector3<int> Vector3i;
  typedef Vector3<float> Vector3f;
  typedef Vector3<double> Vector3d;


}

#endif