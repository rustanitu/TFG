#include <math/MUtils.h>

#include <lqc/lqcdefines.h>
#include <cmath>
#include <cstdarg>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include <math/Vector4.h>
#include <math/Matrix4.h>

namespace lqc
{
  double GetPI ()
  {
    return LQC_PI;
  }

  double cotan (double angle)
  {
    return 1.0 / tan (angle);
  }

  double Cotangent (double angle)
  {
    return cotan (angle);
  }

  double DegreesToRadians (double degrees)
  {
    return degrees * (PI / 180.0);
  }

  double RadiansToDegrees (double radians)
  {
    return radians * (180.0 / PI);
  }

  int Clamp (int value, int lower_bound, int higher_bound)
  {
    if (value < lower_bound)
      value = lower_bound;
    if (value > higher_bound)
      value = higher_bound;

    return value;
  }

  float Clamp (float value, float lower_bound, float higher_bound)
  {
    if (value < lower_bound)
      value = lower_bound;
    if (value > higher_bound)
      value = higher_bound;

    return value;
  }

  bool IsNaN (double x)
  {
    return x != x;
  }

  float Constrain (float x, float a, float b)
  {
    if (x < a)
      return a;
    if (x > b)
      return b;
    return x;
  }

  float Q_rsqrt (float number)
  {
    long i;
    float x2, y;
    const float threehalfs = 1.5f;

    x2 = number * 0.5F;
    y = number;
    i = *(long *)&y;                              // evil floating point bit level hacking
    i = 0x5f3759df - (i >> 1);                    // what the fuck?
    y = *(float *)&i;
    y = y * (threehalfs - (x2 * y * y));          // 1st iteration
    //y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

    return y;
  }

  Matrix4f MultiplyMatrices (const Matrix4f* m1, const Matrix4f* m2)
  {
    Matrix4f out = IDENTITY_MATRIX;
    unsigned int row, column, row_offset;

    for (row = 0, row_offset = row * 4; row < 4; ++row, row_offset = row * 4)
    for (column = 0; column < 4; ++column)
      out.m[row_offset + column] =
      (m1->m[row_offset + 0] * m2->m[column + 0]) +
      (m1->m[row_offset + 1] * m2->m[column + 4]) +
      (m1->m[row_offset + 2] * m2->m[column + 8]) +
      (m1->m[row_offset + 3] * m2->m[column + 12]);

    return out;
  }

  void RotateAboutX (Matrix4f* m, float angle)
  {
    Matrix4f rotation = IDENTITY_MATRIX;
    float sine = (float)sin (angle);
    float cosine = (float)cos (angle);

    rotation.m[5] = cosine;
    rotation.m[6] = -sine;
    rotation.m[9] = sine;
    rotation.m[10] = cosine;

    memcpy (m->m, MultiplyMatrices (m, &rotation).m, sizeof(m->m));
  }

  void RotateAboutY (Matrix4f* m, float angle)
  {
    Matrix4f rotation = IDENTITY_MATRIX;
    float sine = (float)sin (angle);
    float cosine = (float)cos (angle);

    rotation.m[0] = cosine;
    rotation.m[8] = sine;
    rotation.m[2] = -sine;
    rotation.m[10] = cosine;

    memcpy (m->m, MultiplyMatrices (m, &rotation).m, sizeof(m->m));
  }

  void RotateAboutZ (Matrix4f* m, float angle)
  {
    Matrix4f rotation = IDENTITY_MATRIX;
    float sine = (float)sin (angle);
    float cosine = (float)cos (angle);

    rotation.m[0] = cosine;
    rotation.m[1] = -sine;
    rotation.m[4] = sine;
    rotation.m[5] = cosine;

    memcpy (m->m, MultiplyMatrices (m, &rotation).m, sizeof(m->m));
  }

  /**
  * counter clockwise rotation
  * |x'| = |cos angle -sin angle| |x|
  * |y'|   |sin angle  cos angle| |y|
  * clockwise rotation
  * |x'| = | cos angle  sin angle| |x|
  * |y'|   |-sin angle  cos angle| |y|
  **/
  void RadiusRotate2D (Vector2f& vec, double angle, bool is_counter_clockwise)
  {
    float x = vec.x;
    float y = vec.y;
    if (is_counter_clockwise)
    {
      vec.x = (float)cos (angle)*x - (float)sin (angle)*y;
      vec.y = (float)sin (angle)*x + (float)cos (angle)*y;
    }
    else
    {
      vec.x = (float)cos (angle)*x + (float)sin (angle)*y;
      vec.y = -(float)sin (angle)*x + (float)cos (angle)*y;
    }
  }

  void ScaleMatrix (Matrix4f* m, float x, float y, float z)
  {
    Matrix4f scale = IDENTITY_MATRIX;

    scale.m[0] = x;
    scale.m[5] = y;
    scale.m[10] = z;

    memcpy (m->m, MultiplyMatrices (m, &scale).m, sizeof(m->m));
  }

  void TranslateMatrix (Matrix4f* m, float x, float y, float z)
  {
    Matrix4f translation = IDENTITY_MATRIX;

    translation.m[12] = x;
    translation.m[13] = y;
    translation.m[14] = z;

    memcpy (m->m, MultiplyMatrices (m, &translation).m, sizeof(m->m));
  }

  Matrix4f CreateTranslateMatrix (float x, float y, float z)
  {
    Matrix4f translation = IDENTITY_MATRIX;
    translation.m[12] = x;
    translation.m[13] = y;
    translation.m[14] = z;
    return translation;
  }

  /*
      xScale, 0,      0,                        0,
      0,      yScale, 0,                        0,
      0,      0,      (far + near) / nearmfar, -1,
      0,      0,      2*far*near / nearmfar,    0 
  */
  Matrix4f CreateProjectionMatrix (float fovy, float aspect_ratio, float near_plane, float far_plane)
  {
    Matrix4f out = { { 0 } };

    const float
      y_scale = (float)Cotangent (DegreesToRadians ((double)fovy / 2.0)),
      x_scale = y_scale / aspect_ratio,
      frustum_length = far_plane - near_plane;

    out.m[0] = x_scale;
    out.m[5] = y_scale;
    out.m[10] = ((far_plane + near_plane) / frustum_length);
    out.m[11] = -1;
    out.m[14] = ((2 * near_plane * far_plane) / frustum_length);

    return out;
  }
  /*
  | 2/(r-l), 0,       0,       -(r+l)/(r-l) |
  | 0,       2/(t-b), 0,       -(t+b)/(t-b) |
  | 0,       0,       2/(f-n), -(f+n)/(f-n) |
  | 0,       0,       0,       1            |
  */
  Matrix4f CreateOrthoMatrix (float l, float r, float b, float t, float near_plane, float far_plane)
  {

    float f = far_plane;
    float n = near_plane;
    
    Matrix4f out = { { 0 } };

    out.m[0] = 2.0f / (r - l);    out.m[3] = - (r + l) / (r - l);
    out.m[5] = 2.0f / (t - b);    out.m[7] = - (t + b) / (t - b);
    out.m[10] = 2.0f / (f - n);  out.m[11] = - (f + n) / (f - n);
    out.m[15] = 1.0f;
    
    return out;
  }

  double Average (int num, ...)
  {
    va_list arguments;
    double sum = 0;

    va_start (arguments, num);
    for (int x = 0; x < num; x++) 
      sum += va_arg (arguments, double);
    va_end (arguments);

    return sum / (double)num;
  }

  Vector4f operator * (const Matrix4f& m, const Vector4f& v)
  {
    printf ("Matata\n");
    return Vector4f (0);
  }
}