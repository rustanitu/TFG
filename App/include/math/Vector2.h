#ifndef LQC_MATH_VECTOR2_H
#define LQC_MATH_VECTOR2_H

#include <cmath>
#include <algorithm>
#include <vector>
#include <iostream>

namespace lqc
{
  template<typename T>
  class Vector2
  {
  public:
    Vector2();
    Vector2(T value);
    Vector2(T X, T Y);

    double Magnitude();
    void Normalize();

    //friend std::ostream& operator <<(std::ostream&, const Vector2<T>&);
    //friend std::istream& operator >>(std::istream&, Vector2<T>&);
    //friend Vector2<T> operator + (const T, const Vector2<T>);
    //friend Vector2<T> operator - (const T, const Vector2<T>);

    Vector2<T>& operator =(const Vector2<T>);

    Vector2<T> operator + (const Vector2<T>) const;
    Vector2<T> operator + (const T) const;
    Vector2<T>& operator ++ ();
    Vector2<T>& operator ++ (int);
    Vector2<T>& operator += (const Vector2<T>);

    Vector2<T> operator - (const Vector2<T>) const;
    Vector2<T> operator - (const T) const;
    Vector2<T>& operator -= (const Vector2<T>);

    Vector2<T> operator * (const Vector2<T>) const;
    Vector2<T> operator * (const T) const;

    Vector2<T> operator / (const Vector2<T>) const;
    Vector2<T> operator / (const T) const;

    T x, y;
  };

  template<typename T>
  std::ostream& operator << (std::ostream& out, const Vector2<T>& v)
  {
    out << v.x << " " << v.y;
    return out;
  }

  template<typename T>
  std::istream& operator >> (std::istream& in, Vector2<T>& v)
  {
    in >> v.x >> v.y;
    return in;
  }

  template<typename T>
  Vector2<T> operator + (const T t, const Vector2<T> v)
  {
    Vector2<T> temp;
    temp.x = v.x + t;
    temp.y = v.y + t;
    return temp;
  }

  template<typename T>
  Vector2<T> operator - (const T t, const Vector2<T> v)
  {
    Vector2<T> temp;
    temp.x = v.x - t;
    temp.y = v.y - t;
    return temp;
  }

  template<typename T>
  bool operator == (const Vector2<T> v, const Vector2<T> u)
  {
    return ((v.x == u.x) && (v.y == u.y));
  }

  template<typename T>
  Vector2<T>& Vector2<T>::operator = (const Vector2<T> v)
  {
    this->x = v.x;
    this->y = v.y;
    return *this;
  }

  template<typename T>
  Vector2<T> Vector2<T>::operator + (const Vector2<T> v) const
  {
    Vector2<T> temp;
    temp.x = this->x + v.x;
    temp.y = this->y + v.y;
    return temp;
  }

  template<typename T>
  Vector2<T> Vector2<T>::operator + (const T t) const
  {
    Vector2<T> temp;
    temp.x = this->x + t;
    temp.y = this->y + t;
    return temp;
  }

  template<typename T>
  Vector2<T>& Vector2<T>::operator ++ ()
  {
    ++(this->x);
    ++(this->y);
    return *this;
  }

  template<typename T>
  Vector2<T>& Vector2<T>::operator ++ (int)
  {
    this->x++;
    this->y++;
    return *this;
  }

  template<typename T>
  Vector2<T>& Vector2<T>::operator += (const Vector2<T> v)
  {
    this->x += v.x;
    this->y += v.y;
    return *this;
  }

  template<typename T>
  Vector2<T> Vector2<T>::operator - (const Vector2<T> v) const
  {
    Vector2<T> temp;
    temp.x = this->x - v.x;
    temp.y = this->y - v.y;
    return temp;
  }

  template<typename T>
  Vector2<T> Vector2<T>::operator - (const T t) const
  {
    Vector2<T> temp;
    temp.x = this->x - t;
    temp.y = this->y - t;
    return temp;
  }

  template<typename T>
  Vector2<T>& Vector2<T>::operator -= (const Vector2<T> v)
  {
    this->x -= v.x;
    this->y -= v.y;
    return *this;
  }

  template<typename T>
  Vector2<T> Vector2<T>::operator * (const Vector2<T> v) const
  {
    Vector2<T> temp;
    temp.x = this->x * v.x;
    temp.y = this->y * v.y;
    return temp;
  }

  template<typename T>
  Vector2<T> Vector2<T>::operator * (const T t) const
  {
    Vector2<T> temp;
    temp.x = this->x * t;
    temp.y = this->y * t;
    return temp;
  }

  template<typename T>
  Vector2<T> Vector2<T>::operator / (const Vector2<T> v) const
  {
    Vector2<T> temp;
    temp.x = this->x / v.x;
    temp.y = this->y / v.y;
    return temp;
  }

  template<typename T>
  Vector2<T> Vector2<T>::operator / (const T t) const
  {
    Vector2<T> temp;
    temp.x = this->x / t;
    temp.y = this->y / t;
    return temp;
  }

  typedef Vector2<int> Vector2i;
  typedef Vector2<float> Vector2f;
  typedef Vector2<double> Vector2d;
}

#endif