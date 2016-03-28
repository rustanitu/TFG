#ifndef LQC_MATH_VECTOR4_H
#define LQC_MATH_VECTOR4_H

#include <iostream>

namespace lqc
{
  template<typename T>
  class Vector4
  {
  public:
    Vector4 ();
    Vector4 (T value);
    Vector4 (T X, T Y, T Z, T W);

    double Magnitude ();
    void Normalize ();
    bool IsZero ();
    void Sqrt ();

    static bool IsEqual (const Vector4<T> v1, const Vector4<T> v2);
    static Vector4<T> Pow (Vector4<T> v, int exponent);
    static Vector4<T> Zero ();
    static Vector4<T> One ();
    static Vector4<T> Multiply (Vector4<T> d, T f);
    static Vector4<T> Normalize (Vector4<T> v);

    //friend std::ostream& operator <<(std::ostream&, const Vector4<T>&);
    //friend std::istream& operator >>(std::istream&, Vector4<T>&);
    //friend Vector4<T> operator + (const T, const Vector4<T>);
    //friend Vector4<T> operator * (const T, const Vector4<T>);

    Vector4<T>& operator =(const Vector4<T>);

    Vector4<T> operator + (const Vector4<T>) const;
    Vector4<T> operator + (const T) const;
    Vector4<T>& operator ++ ();
    Vector4<T>& operator ++ (int);
    Vector4<T>& operator += (const Vector4<T>);
    
    Vector4<T> operator - (const Vector4<T>) const;
    Vector4<T> operator - (const T) const;
    Vector4<T>& operator -= (const Vector4<T>);
    
    Vector4<T> operator * (const Vector4<T>) const;
    Vector4<T> operator * (const T) const;

    Vector4<T> operator / (const Vector4<T>) const;
    Vector4<T> operator / (const T) const;
    
    bool operator == (const Vector4<T> v);
    bool operator != (const Vector4<T> v);

    T x, y, z, w;
  };
  
  template<typename T>
  std::ostream& operator << (std::ostream& out, const Vector4<T>& v)
  {
    out << v.x << " " << v.y << " " << v.z << " " << v.w;
    return out;
  }

  template<typename T>
  std::istream& operator >> (std::istream& in, Vector4<T>& v)
  {
    in >> v.x >> v.y >> v.z >> v.w;
    return in;
  }

  template<typename T>
  Vector4<T> operator + (const T t, const Vector4<T> v)
  {
    Vector4<T> temp;
    temp.x = v.x + t;
    temp.y = v.y + t;
    temp.z = v.z + t;
    temp.w = v.w + t;
    return temp;
  }

  template<typename T>
  Vector4<T> operator * (const T t, const Vector4<T> v)
  {
    Vector4<T> temp;
    temp.x = v.x * t;
    temp.y = v.y * t;
    temp.z = v.z * t;
    temp.w = v.w * t;
    return temp;
  }
  
  template<typename T>
  Vector4<T>& Vector4<T>::operator = (const Vector4<T> v)
  {
    this->x = v.x;
    this->y = v.y;
    this->z = v.z;
    this->w = v.w;
    return *this;
  }

  template<typename T>
  Vector4<T> Vector4<T>::operator + (const Vector4<T> v) const
  {
    Vector4<T> temp;
    temp.x = this->x + v.x;
    temp.y = this->y + v.y;
    temp.z = this->z + v.z;
    temp.w = this->w + v.w;
    return temp;
  }

  template<typename T>
  Vector4<T> Vector4<T>::operator + (const T t) const
  {
    Vector4<T> temp;
    temp.x = this->x + t;
    temp.y = this->y + t;
    temp.z = this->z + t;
    temp.w = this->w + t;
    return temp;
  }

  template<typename T>
  Vector4<T>& Vector4<T>::operator ++ ()
  {
    ++(this->x);
    ++(this->y);
    ++(this->z);
    ++(this->w);
    return *this;
  }

  template<typename T>
  Vector4<T>& Vector4<T>::operator ++ (int)
  {
    this->x++;
    this->y++;
    this->z++;
    this->w++;
    return *this;
  }

  template<typename T>
  Vector4<T>& Vector4<T>::operator += (const Vector4<T> v)
  {
    this->x += v.x;
    this->y += v.y;
    this->z += v.z;
    this->w += v.w;
    return *this;
  }

  template<typename T>
  Vector4<T> Vector4<T>::operator - (const Vector4<T> v) const
  {
    Vector4<T> temp;
    temp.x = this->x - v.x;
    temp.y = this->y - v.y;
    temp.z = this->z - v.z;
    temp.w = this->w - v.w;
    return temp;
  }

  template<typename T>
  Vector4<T> Vector4<T>::operator - (const T t) const
  {
    Vector4<T> temp;
    temp.x = this->x - t;
    temp.y = this->y - t;
    temp.z = this->z - t;
    temp.w = this->w - t;
    return temp;
  }

  template<typename T>
  Vector4<T>& Vector4<T>::operator -= (const Vector4<T> v)
  {
    this->x -= v.x;
    this->y -= v.y;
    this->z -= v.z;
    this->w -= v.w;
    return *this;
  }

  template<typename T>
  Vector4<T> Vector4<T>::operator * (const Vector4<T> v) const
  {
    Vector4<T> temp;
    temp.x = this->x * v.x;
    temp.y = this->y * v.y;
    temp.z = this->z * v.z;
    temp.w = this->w * v.w;
    return temp;
  }

  template<typename T>
  Vector4<T> Vector4<T>::operator * (const T t) const
  {
    Vector4<T> temp;
    temp.x = this->x * t;
    temp.y = this->y * t;
    temp.z = this->z * t;
    temp.w = this->w * t;
    return temp;
  }

  template<typename T>
  Vector4<T> Vector4<T>::operator / (const Vector4<T> v) const
  {
    Vector4<T> temp;
    temp.x = this->x / v.x;
    temp.y = this->y / v.y;
    temp.z = this->z / v.z;
    temp.w = this->w / v.w;
    return temp;
  }

  template<typename T>
  Vector4<T> Vector4<T>::operator / (const T t) const
  {
    Vector4<T> temp;
    temp.x = this->x / t;
    temp.y = this->y / t;
    temp.z = this->z / t;
    temp.w = this->w / t;
    return temp;
  }

  template<typename T>
  bool Vector4<T>::operator == (const Vector4<T> v)
  {
    return (this->x == v.x) && (this->y == v.y)
      && (this->z == v.z) && (this->w == v.w);
  }

  template<typename T>
  bool Vector4<T>::operator != (const Vector4<T> v)
  {
    return !((this->x == v.x) && (this->y == v.y)
      && (this->z == v.z) && (this->w == v.w));
  }

  typedef Vector4<int> Vector4i;
  typedef Vector4<float> Vector4f;
  typedef Vector4<double> Vector4d;
}
#endif