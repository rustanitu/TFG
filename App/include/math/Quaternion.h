//Links:
//http://content.gpwiki.org/index.php/OpenGL:Tutorials:Using_Quaternions_to_represent_rotation
//http://www.gamedev.net/page/resources/_/technical/math-and-physics/a-simple-quaternion-based-camera-r1997
//http://www.flipcode.com/documents/matrfaq.html

#ifndef LQC_MATH_QUATERNION_H
#define LQC_MATH_QUATERNION_H

#include <math/Vector3.h>
#include <math/Matrix4.h>
#include <cmath>
#include <cstdio>
#include <iostream>

namespace lqc
{
  class Quaternion
  {
  public:
    static float Length (Quaternion q);
    static Quaternion Normalize (Quaternion q);
    static Quaternion Conjugate (Quaternion q);
    static Quaternion GetConjugate (Quaternion q);
    static Quaternion Identity ();
    static Quaternion Multiply (Quaternion A, Quaternion B);
    static Vector3f Multiply (Quaternion Q, Vector3f B);
    static Matrix4f GetRotationMatrix (Vector3f A, float angle);
  
  public:
    void Normalize ();
    void Conjugate ();
    void Multiply (Quaternion q);
    void Inverse ();

    Matrix4f GetMatrix ();
    Matrix4f ToMatrix ();

    void FromEuler (float pitch, float yaw, float roll);

    // Convert to Axis/Angles
    void GetAxisAngle (Vector3f* axis, float* angle);

    //Rotate the Quaternion around the axis 'v'
    // q = cos(angle / 2) + v * sin (angle / 2)
    void FromAxis (const Vector3f v, float angle);

    void FromMatrix (const Matrix4f m);
    void FromAxis (const Vector3f x, const Vector3f y, const Vector3f z);
  
  public:
    Quaternion (float xi = 0.0f, float yi = 0.0f, float zi = 0.0f, float wi = 1.0f);
    Quaternion (Vector3f v, float wi);
    ~Quaternion ();

    float x, y, z, w;

  public:
    Quaternion& operator =(const Quaternion q)
    {
      this->x = q.x;
      this->y = q.y;
      this->z = q.z;
      this->w = q.w;
      return *this;
    }

    Quaternion operator + (const Quaternion q) const
    {
      Quaternion temp;
      temp.x = this->x + q.x;
      temp.y = this->y + q.y;
      temp.z = this->z + q.z;
      temp.w = this->w + q.w;
      return temp;
    }

    Quaternion Quaternion::operator - (const Quaternion q) const
    {
      Quaternion temp;
      temp.x = this->x - q.x;
      temp.y = this->y - q.y;
      temp.z = this->z - q.z;
      temp.w = this->w - q.w;
      return temp;
    }

    Quaternion Quaternion::operator * (const Quaternion q) const
    {
      return Quaternion::Multiply (*this, q);
    }

    Vector3f Quaternion::operator * (const Vector3f vec) const
    {
      Quaternion Q = *this;
      Vector3f vn (vec);
      vn.Normalize ();

      Quaternion vecQuat, resQuat;
      vecQuat.x = vn.x;
      vecQuat.y = vn.y;
      vecQuat.z = vn.z;
      vecQuat.w = 0.0f;

      resQuat = vecQuat * GetConjugate (Q);
      resQuat = Q * resQuat;

      return (Vector3f (resQuat.x, resQuat.y, resQuat.z));
    }
  };
}

#endif