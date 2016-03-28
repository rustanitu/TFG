#include <math/MGeometry.h>

#include <cmath>
#include <lqc/lqcdefines.h>

namespace lqc
{
  float Dot (Vector2f v1, Vector2f v2)
  {
    return v1.x*v2.x + v1.y*v2.y;
  }

  float Dot (Vector3f v1, Vector3f v2)
  {
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
  }

  float DotProduct (Vector2f v1, Vector2f v2)
  {
    return Dot (v1, v2);
  }

  float DotProduct (Vector3f v1, Vector3f v2)
  {
    return Dot (v1, v2);
  }

  float Magnitude (Vector2f v)
  {
    return sqrt (v.x*v.x + v.y*v.y);
  }

  float Magnitude (Vector3f v)
  {
    return sqrt (v.x*v.x + v.y*v.y + v.z*v.z);
  }

  /**
  * Cross Product between 2 2D Vectors
  *
  *         ( u.x v.x i )
  * u x v = ( u.y v.y j )
  *         ( 0.0 0.0 k )
  *
  * u x v = (u.y*0.0 - 0.0*v.y)i + (0.0*v.x - u.x*0.0)j + (u.x*v.y - u.y*v.x)k
  * u x v = (u.x*v.y - u.y*v.x)k
  */
  float Cross (Vector2f u, Vector2f v)
  {
    return u.x*v.y - u.y*v.x;
  }

  /**
  * Cross Product between 2 3D Vectors
  *
  *         ( u.x v.x i )
  * u x v = ( u.y v.y j )
  *         ( u.z v.z k )
  *
  * u x v = (u.y*v.z - u.z*v.y)i + (u.z*v.x - u.x*v.z)j + (u.x*v.y - u.y*v.x)k
  */
  Vector3f Cross (Vector3f u, Vector3f v)
  {
    return Vector3f (u.y*v.z - u.z*v.y, u.z*v.x - u.x*v.z, u.x*v.y - u.y*v.x);
  }

  float CrossProduct (Vector2f u, Vector2f v)
  {
    return Cross (u, v);
  }

  Vector3f CrossProduct (Vector3f u, Vector3f v)
  {
    return Cross (u, v);
  }

  float Distance (Vector2f v1, Vector2f v2)
  {
    return sqrt (
      ((v2.x - v1.x)*(v2.x - v1.x)) +
      ((v2.y - v1.y)*(v2.y - v1.y)));
  }

  float Distance (Vector3f v)
  {
    return sqrt ((v.x*v.x) + (v.y*v.y) + (v.z*v.z));
  }

  float Distance (Vector3f v1, Vector3f v2)
  {
    return sqrt (
      ((v2.x - v1.x)*(v2.x - v1.x)) +
      ((v2.y - v1.y)*(v2.y - v1.y)) +
      ((v2.z - v1.z)*(v2.z - v1.z)));
  }

  double Distance (Vector3d v1, Vector3d v2)
  {
    return sqrt (
      ((v2.x - v1.x)*(v2.x - v1.x)) +
      ((v2.y - v1.y)*(v2.y - v1.y)) +
      ((v2.z - v1.z)*(v2.z - v1.z)));
  }

  double Distance (double d1, double d2)
  {
    return std::abs (d2 - d1);
  }

  double DistanceXYZ (Vector4d v1, Vector4d v2)
  {
    return sqrt (
      ((v2.x - v1.x)*(v2.x - v1.x)) +
      ((v2.y - v1.y)*(v2.y - v1.y)) +
      ((v2.z - v1.z)*(v2.z - v1.z)));
  }

  float SquareDistance (Vector2f v)
  {

    return SQR (v.x) + SQR(v.y);
  }

  float SquareDistance (Vector2f p1, Vector2f p2)
  {

    return SQR (p2.x - p1.x) + SQR (p2.y - p1.y);
  }

  float SquareDistance (Vector3f p1, Vector3f p2)
  {
    return SQR (p2.x - p1.x) + SQR (p2.y - p1.y) + SQR (p2.z - p1.z);
  }

  float SquareDistance (Vector3d p1, Vector3d p2)
  {
    return SQR (p2.x - p1.x) + SQR (p2.y - p1.y) + SQR (p2.z - p1.z);
  }

  float DistanceManhattan (Vector3f a, Vector3f b)
  {
    return fabs (a.x - b.x) + fabs (a.y - b.y) + fabs (a.z - b.z);
  }

  double DistanceManhattan (Vector3d a, Vector3d b)
  {
    return fabs (a.x - b.x) + fabs (a.y - b.y) + fabs (a.z - b.z);
  }

  int Orientation (Vector2f v1, Vector2f v2)
  {
    float val = v1.y * v2.x - v1.x * v2.y;
    if (val == 0.0f) return 0;
    return (val > 0.0f) ? 1 : 2;
  }

  float AreaOfTriangle (float AB_lenght, float BC_lenght, float CA_lenght)
  {
    float a = AB_lenght; float b = BC_lenght; float c = CA_lenght;
    float s = (a + b + c) * 0.5f;
    return sqrt (s*(s - a)*(s - b)*(s - c));
  }

  float AreaOfTriangle (float b, float h)
  {
    return 0.5f * b * h;
  }

  //http://justbasic.wikispaces.com/Check+for+collision+of+two+circles,+get+intersection+points
  bool Intersection (std::vector<Vector2f>& points, Circle2f c1, Circle2f c2)
  {
    //This function checks for the intersection of two circles.
    //If one circle is wholly contained within the other a -1 is returned
    //If there is no intersection of the two circles a 0 is returned
    //If the circles intersect a 1 is returned and
    //the coordinates are placed in xi1, yi1, xi2, yi2

    float r0 = c1.radius;
    float x0 = c1.center.x;
    float y0 = c1.center.y;
    float r1 = c2.radius;
    float x1 = c2.center.x;
    float y1 = c2.center.y;


    //dx and dy are the vertical And horizontal distances between
    //the circle centers.
    float dx = x1 - x0;
    float dy = y1 - y0;

    //Determine the straight-Line distance between the centers.
    float d = sqrt ((dy*dy) + (dx*dx));

    //Check for solvability.
    if (d > (r0 + r1))
    {
      printf ("lqc: No solution. circles do Not intersect.\n");
      return false;
    }

    if (d < abs (r0 - r1))
    {
      printf ("lqc: No solution. one circle is contained in the other.\n");
      return false;
    }

    // 'point 2' is the point where the Line through the circle
    // intersection points crosses the Line between the circle
    // centers.

    // Determine the distance from point 0 To point 2.
    double a = ((r0*r0) - (r1*r1) + (d*d)) / (2.0 * d);

    //Determine the coordinates of point 2.
    float x2 = x0 + (dx * (float)a / d);
    float y2 = y0 + (dy * (float)a / d);

    //Determine the distance from point 2 To either of the
    //intersection points.
    float h = sqrt ((r0*r0) - ((float)a*(float)a));

    // Now determine the offsets of the intersection points from
    // point 2.
    float rx = (0 - dy) * (h / d);
    float ry = dx * (h / d);

    // Determine the absolute intersection points.
    float xi1 = x2 + rx;
    float xi2 = x2 - rx;
    float yi1 = y2 + ry;
    float yi2 = y2 - ry;

    points.push_back (Vector2f (xi1, yi1));
    points.push_back (Vector2f (xi2, yi2));

    //printf("p1 = %.2f %.2f, p2 = %.2f %.2f\n", xi1, yi1, xi2, yi2);
    return true;
  }
}