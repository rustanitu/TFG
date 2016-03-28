#include <math/Quaternion.h>
#include <lqc/lqcdefines.h>

namespace lqc
{
  float Quaternion::Length (Quaternion q)
  {
    return sqrt (q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
  }

  Quaternion Quaternion::Normalize (Quaternion q)
  {
    float L = Length (q);
    if (L == 0.0f)
    {
      q.w = 1.0f;
      q.x = q.y = q.z = 0.0f;
    }
    else
    {
      q.x /= L; q.y /= L;
      q.z /= L; q.w /= L;
    }
    return q;
  }

  Quaternion Quaternion::Conjugate (Quaternion q)
  {
    q.x = -q.x;
    q.y = -q.y;
    q.z = -q.z;
    return q;
  }

  Quaternion Quaternion::GetConjugate (Quaternion q)
  {
    return Quaternion (-q.x, -q.y, -q.z, q.w);
  }

  Quaternion Quaternion::Identity ()
  {
    Quaternion q;
    return q;
  }

  Quaternion Quaternion::Multiply (Quaternion A, Quaternion B)
  {
    Quaternion C;
    //Left order
    C.w = A.w*B.w - A.x*B.x - A.y*B.y - A.z*B.z;
    C.x = A.w*B.x + A.x*B.w - A.y*B.z + A.z*B.y;
    C.y = A.w*B.y + A.x*B.z + A.y*B.w - A.z*B.x;
    C.z = A.w*B.z - A.x*B.y + A.y*B.x + A.z*B.w;
    
    //Right order
    //C.w = A.w*B.w - A.x*B.x - A.y*B.y - A.z*B.z;
    //C.x = A.w*B.x + A.x*B.w + A.y*B.z - A.z*B.y;
    //C.y = A.w*B.y - A.x*B.z + A.y*B.w + A.z*B.x;
    //C.z = A.w*B.z + A.x*B.y - A.y*B.x + A.z*B.w;
    return C;
  }

  Vector3f Quaternion::Multiply (Quaternion Q, Vector3f B)
  {
    B.Normalize ();

    Quaternion vecQuat, resQuat;
    vecQuat.x = B.x;
    vecQuat.y = B.y;
    vecQuat.z = B.z;
    vecQuat.w = 0.0f;

    resQuat = Quaternion::Multiply (vecQuat, Quaternion::GetConjugate (Q));
    resQuat = Quaternion::Multiply (Q, resQuat);

    return (Vector3f (resQuat.x, resQuat.y, resQuat.z));
  }

  Matrix4f Quaternion::GetRotationMatrix (Vector3f A, float angle)
  {
    return lqc::IDENTITY_MATRIX;

  }

  void Quaternion::Normalize ()
  {
    float L = Length (*this);
    if (L == 0.0f)
    {
      w = 1.0f;
      x = y = z = 0.0f;
    }
    else
    {
      x /= L; y /= L;
      z /= L; w /= L;
    }
  }

  void Quaternion::Conjugate ()
  {
    x = -x;
    y = -y;
    z = -z;
  }

  void Quaternion::Multiply (Quaternion q)
  {
    w = w*q.x + x*q.w + y*q.z - z*q.y;
    x = w*q.y - x*q.z + y*q.w + z*q.x;
    y = w*q.z + x*q.y - y*q.x + z*q.w;
    z = w*q.w - x*q.x - y*q.y - z*q.z;
  }

  void Quaternion::Inverse ()
  {
    x *= -1.0f;
    y *= -1.0f;
    z *= -1.0f;

    Normalize ();
  }

  //http://www.arcsynthesis.org/gltut/Positioning/Tut08%20Quaternions.html
  Matrix4f Quaternion::GetMatrix ()
  {
    float xx = x * x;
    float yy = y * y;
    float zz = z * z;
    float ww = w * w;

    float xy = x * y;
    float xz = x * z;
    float xw = x * w;

    float yz = y * z;
    float yw = y * w;

    float zw = z * w;

    float c = 2.0f / (float)(sqrt (ww + xx + yy + zz));

    //Matrix4f m = { {
    //    1.0f - c * (yy + zz),        c * (xy - zw),        c * (xz + yw), 0.0f,
    //           c * (xy + zw), 1.0f - c * (xx + zz),        c * (yz - xw), 0.0f,
    //           c * (xz - yw),        c * (yz + xw), 1.0f - c * (xx + yy), 0.0f,
    //                    0.0f,                 0.0f,                 0.0f, 1.0f
    //  } };

    Matrix4f m = { {
        1.0f - c * (yy + zz), c * (xy + zw), c * (xz - yw), 0.0f,
        c * (xy - zw), 1.0f - c * (xx + zz), c * (yz + xw), 0.0f,
        c * (xz + yw), c * (yz - xw), 1.0f - c * (xx + yy), 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
      } };
    return m;
  }

  Matrix4f Quaternion::ToMatrix ()
  {
    return GetMatrix ();
  }

  void Quaternion::FromEuler (float pitch, float yaw, float roll)
  {
    // Basically we create 3 Quaternions, one for pitch, one for yaw, one for roll
    // and multiply those together.
    // the calculation below does the same, just shorter

    float p = pitch * PIOVER180 / 2.0;
    float y = yaw * PIOVER180 / 2.0;
    float r = roll * PIOVER180 / 2.0;

    float sinp = sin (p);
    float siny = sin (y);
    float sinr = sin (r);
    float cosp = cos (p);
    float cosy = cos (y);
    float cosr = cos (r);

    this->x = sinr * cosp * cosy - cosr * sinp * siny;
    this->y = cosr * sinp * cosy + sinr * cosp * siny;
    this->z = cosr * cosp * siny - sinr * sinp * cosy;
    this->w = cosr * cosp * cosy + sinr * sinp * siny;

    Normalize ();
  }

  // Convert to Axis/Angles
  void Quaternion::GetAxisAngle (Vector3f* axis, float* angle)
  {
    float scale = sqrt (x * x + y * y + z * z);
    axis->x = x / scale;
    axis->y = y / scale;
    axis->z = z / scale;
    *angle = acos (w) * 2.0f;
  }

  void Quaternion::FromAxis (const Vector3f v, float angle)
  {
    Vector3f A = v;
    float half_angle = DEGREE_TO_RADS(angle) / 2.0f;
    float sin_v = sinf (half_angle);

    x = A.x * sin_v;
    y = A.y * sin_v;
    z = A.z * sin_v;
    w = cos(half_angle);

    Normalize ();
  }

  //http://en.wikipedia.org/wiki/Rotation_formalisms_in_three_dimensions
  //DEVE TÊ ALGUMA PORRA AQUI
  void Quaternion::FromMatrix (const Matrix4f mat)
  {
    float m[4][4];

    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
      {
        m[i][j] = mat.m[i * 4 + j];
      }

    float s = 0.0f;
    float q[4] = { 0.0f };
    float trace = m[0][0] + m[1][1] + m[2][2];

    if (trace > 0.0f)
    {
      s = sqrtf (trace + 1.0f);
      q[3] = s * 0.5f;
      s = 0.5f / s;
      q[0] = (m[1][2] - m[2][1]) * s;
      q[1] = (m[2][0] - m[0][2]) * s;
      q[2] = (m[0][1] - m[1][0]) * s;
    }
    else
    {
      int nxt[3] = { 1, 2, 0 };
      int i = 0, j = 0, k = 0;

      if (m[1][1] > m[0][0])
        i = 1;

      if (m[2][2] > m[i][i])
        i = 2;

      j = nxt[i];
      k = nxt[j];
      s = sqrtf ((m[i][i] - (m[j][j] + m[k][k])) + 1.0f);

      q[i] = s * 0.5f;
      s = 0.5f / s;
      q[3] = (m[j][k] - m[k][j]) * s;
      q[j] = (m[i][j] + m[j][i]) * s;
      q[k] = (m[i][k] + m[k][i]) * s;
    }

    x = q[0], y = q[1], z = q[2], w = q[3];
  }

  void Quaternion::FromAxis (const Vector3f x, const Vector3f y, const Vector3f z)
  {

  }

  Quaternion::Quaternion (float xi, float yi, float zi, float wi)
    : x (xi), y (yi), z (zi), w (wi)
  {}

  Quaternion::Quaternion (Vector3f v, float wi)
    : x (v.x), y (v.y), z (v.z), w (wi)
  {}

  Quaternion::~Quaternion ()
  {}
}