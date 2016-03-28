#ifndef LQC_MATH_GEOMETRY_H
#define LQC_MATH_GEOMETRY_H

#include <math/Vector2.h>
#include <math/Vector3.h>
#include <math/Vector4.h>
#include <math/Circle.h>

namespace lqc
{
  float Dot (Vector2f v1, Vector2f v2);
  float Dot (Vector3f v1, Vector3f v2);
  float DotProduct (Vector2f v1, Vector2f v2);
  float DotProduct (Vector3f v1, Vector3f v2);

  float Magnitude (Vector2f v);
  float Magnitude (Vector3f v);

  float Cross (Vector2f u, Vector2f v);
  Vector3f Cross (Vector3f u, Vector3f v);
  float CrossProduct (Vector2f u, Vector2f v);
  Vector3f CrossProduct (Vector3f u, Vector3f v);

  float Distance (Vector2f v1, Vector2f v2);
  float Distance (Vector3f v);
  float Distance (Vector3f v1, Vector3f v2);
  double Distance (Vector3d v1, Vector3d v2);
  double Distance (double d1, double d2);
  double DistanceXYZ (Vector4d v1, Vector4d v2);

  float SquareDistance (Vector2f v);
  float SquareDistance (Vector2f p1, Vector2f p2);
  float SquareDistance (Vector3f p1, Vector3f p2);
  float SquareDistance (Vector3d p1, Vector3d p2);

  float DistanceManhattan (Vector3f p1, Vector3f p2);
  double DistanceManhattan (Vector3d p1, Vector3d p2);

  /**
  * 0 -> Colinear
  * 1 -> ClockWise
  * 2 -> CounterClockwise
  *
  *         ^ v2
  *        /
  *       /
  *      /
  *     / <---
  *    /      |  Orientation (v1 to v2)
  *   /       |
  *   -----------------> v1
  *
  */
  int Orientation (Vector2f v1, Vector2f v2);

  float AreaOfTriangle (float AB_lenght, float BC_lenght, float CA_lenght);
  float AreaOfTriangle (float b, float h);

  bool Intersection (std::vector<Vector2f>& points, Circle2f c1, Circle2f c2);
}

#endif