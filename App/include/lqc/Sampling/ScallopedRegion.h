//Link http://www.cs.virginia.edu/~gfx/pubs/antimony/
//Credits to Daniel Dunbar & Greg Humphreys

#ifndef LQC_SAMPLING_SCALLOPEDREGION_H
#define LQC_SAMPLING_SCALLOPEDREGION_H

#include <math/Vector2.h>

namespace lqc
{
  typedef struct {
  //center
  // Vector2f or [d, theta]
  Vector2f P;
  float polar_d, polar_theta;
  //radius and sign k {-1, 1} (selects the near or far side of the circle)
  float r, sign;

  float integralAtStart;
  //square of r and polar_d
  float r_SQR, d_SQR;
  } CircularArc;

  class ScallopedSector
  {
  public:
  Vector2f P;
  //angle 1 and angle 2 | area of sector
  float a1, a2, area;

  //Circular arcs
  CircularArc arcs[2];
  public:
  ScallopedSector (Vector2f Pt, float a1, float a2, Vector2f P1, float r1, float sign1, Vector2f P2, float r2, float sign2);

  float CanonizeAngle (float angle);
  void DistToCircle (float angle, Vector2f &C, float r, float *d1_out, float *d2_out);

  void SubtractDisk (Vector2f &C, float r, std::vector<ScallopedSector> *regions);

  float IntegralOfDistToCircle (float x, float d, float r, float k);
  float CalcAreaToAngle (float angle);
  float CalcAngleForArea (float area);

  //funções g(a) e h(a)
  float DistToCurve (float angle, int index);

  //Sample a random point in this sector
  Vector2f Sample ();
  };

  class ScallopedRegion
  {
  public:
  ScallopedRegion (Vector2f P, float r1, float r2, float min_area = .00000001);
  ~ScallopedRegion ();

  bool IsEmpty ();

  void SubtractDisk (Vector2f C, float r);

  Vector2f Sample ();

  public:
  std::vector<ScallopedSector> *m_sectors;
  float minArea;
  float area;
  };
}

#endif