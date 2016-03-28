#ifndef LQC_MATH_T_VECTOR_H
#define LQC_MATH_T_VECTOR_H

#include <cstdlib>
#include <cassert>

namespace lqc
{
  template<typename T>
  class Vector
  {
  public:
    Vector ()
      : m_size (0)
    {
      v = NULL;
    }

    Vector (int size)
      : m_size (size)
    {
      v = new T[size] ();
    }

    ~Vector ()
    {
      delete[] v;
    }

    int GetSize ()
    {
      return m_size;
    }

    bool Set (T value, int index)
    {
      if (IsOutOfBounds (index))
        return false;
      v[index] = value;
    }

    T Get (int index)
    {
      if (IsOutOfBounds (index))
        return -1.f;
      return v[index];
    }
    
    bool IsOutOfBounds (int index)
    {
      return (index < 0 || index >= m_size);
    }
   
    T& at (int idx)
    {
      assert (!IsOutOfBounds (idx));
      return v[idx];
    }

    const T& at (int idx) const
    {
      assert (!IsOutOfBounds (idx));
      return v[idx];
    }

    T& operator() (int idx)
    {
      assert (!IsOutOfBounds (idx));
      return v[idx];
    }

    const T& operator() (int idx) const
    {
      assert (!IsOutOfBounds (idx));
      return v[idx];
    }

  private:
    int m_size;
    T* v;

  public:
    T& operator[] (int idx)
    {
      assert (!IsOutOfBounds (idx));
      return v[idx];
    }

    const T& operator[] (int idx) const
    {
      assert (!IsOutOfBounds (idx));
      return v[idx];
    }
  };

  template<typename T, unsigned int N>
  class Vec
  {
  public:
    Vec ()
      : m_size (N)
    {}

    ~Vec ()
    {}

    int GetSize ()
    {
      return m_size;
    }

    bool Set (T value, int index)
    {
      if (IsOutOfBounds (index))
        return false;
      v[index] = value;
    }

    T Get (int index)
    {
      if (IsOutOfBounds (index))
        return -1.f;
      return v[index];
    }

    bool IsOutOfBounds (int index)
    {
      return (index < 0 || index >= m_size);
    }

    T& at (int idx)
    {
      assert (!IsOutOfBounds (idx));
      return v[idx];
    }

    const T& at (int idx) const
    {
      assert (!IsOutOfBounds (idx));
      return v[idx];
    }

    T& operator() (int idx)
    {
      assert (!IsOutOfBounds (idx));
      return v[idx];
    }

    const T& operator() (int idx) const
    {
      assert (!IsOutOfBounds (idx));
      return v[idx];
    }

  private:
    int m_size;
    T v[N];

  public:
    T& operator[] (int idx)
    {
      assert (!IsOutOfBounds (idx));
      return v[idx];
    }

    const T& operator[] (int idx) const
    {
      assert (!IsOutOfBounds (idx));
      return v[idx];
    }
  };
}

#endif