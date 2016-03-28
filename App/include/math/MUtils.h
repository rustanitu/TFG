#ifndef LQC_MATH_UTILS_H
#define LQC_MATH_UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <time.h>

#include <math/Matrix4.h>
#include <math/Vector2.h>

namespace lqc
{
  double GetPI ();

  double cotan (double angle);

  double Cotangent (double angle);

  double DegreesToRadians (double degrees);
  double RadiansToDegrees (double radians);

  int Clamp (int value, int lower_bound, int higher_bound);
  float Clamp (float value, float lower_bound, float higher_bound);

  bool IsNaN (double x);

  /**
  * Returns:
  * x: if x is between a and b
  * a: if x is less than a
  * b: if x is greater than b
  **/
  float Constrain (float x, float a, float b);

  //http://en.wikipedia.org/wiki/Fast_inverse_square_root
  float Q_rsqrt (float number);

  Matrix4f MultiplyMatrices (const Matrix4f* m1, const Matrix4f* m2);
  void RotateAboutX (Matrix4f* m, float angle);
  void RotateAboutY (Matrix4f* m, float angle);
  void RotateAboutZ (Matrix4f* m, float angle);
  void RadiusRotate2D (Vector2f& vec, double angle, bool is_counter_clockwise = true);
  void ScaleMatrix (Matrix4f* m, float x, float y, float z);
  void TranslateMatrix (Matrix4f* m, float x, float y, float z);

  Matrix4f CreateTranslateMatrix (float x, float y, float z);
  Matrix4f CreateProjectionMatrix (float fovy, float aspect_ratio, float near_plane, float far_plane);
  Matrix4f CreateOrthoMatrix (float l, float r, float b, float t, float near_plane, float far_plane);

  //example: Average ( 2, 10.0, 4.0 ) 
  double Average (int num, ...);
}

#endif