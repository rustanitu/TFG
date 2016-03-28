#define _USE_MATH_DEFINES
#include <math.h>
#include <lqc/Sampling/ScallopedRegion.h>
#include <lqc/Utils/Utils.h>
#include <float.h>

#include <algorithm>

namespace lqc
{
  static const float kTwoPi = (float)(M_PI * 2);

  static float integralOfDistToCircle (float x, float d, float r, float k)
  {
    if (r < FLT_EPSILON)
      return 0.0;

    float sin_x = sin (x);
    float d_sin_x = d*sin_x;
    float y = sin_x*d / r;
    if (y<-1) y = -1;
    else if (y>1) y = 1;

    float theta = asin (y);

    return (r*(r*(x +
      k*theta) +
      k*cos (theta)*d_sin_x) +
      d*cos (x)*d_sin_x)*.5f;
  }

  ScallopedSector::ScallopedSector (Vector2f _Pt, float _a1, float _a2, Vector2f P1, float r1, float sign1, Vector2f P2, float r2, float sign2)
  {
    Vector2f v1 = Vector2f (P1.x - _Pt.x, P1.y - _Pt.y);
    Vector2f v2 = Vector2f (P2.x - _Pt.x, P2.y - _Pt.y);

    P = _Pt;
    a1 = _a1;
    a2 = _a2;

    arcs[0].P = P1;
    arcs[0].r = r1;
    arcs[0].sign = sign1;
    arcs[0].polar_d = sqrt (v1.x*v1.x + v1.y*v1.y);
    arcs[0].r_SQR = arcs[0].r*arcs[0].r;
    arcs[0].d_SQR = arcs[0].polar_d*arcs[0].polar_d;
    arcs[0].polar_theta = atan2 (v1.y, v1.x);
    arcs[0].integralAtStart = integralOfDistToCircle (a1 - arcs[0].polar_theta, arcs[0].polar_d, arcs[0].r, arcs[0].sign);

    arcs[1].P = P2;
    arcs[1].r = r2;
    arcs[1].sign = sign2;
    arcs[1].polar_d = sqrt (v2.x*v2.x + v2.y*v2.y);
    arcs[1].r_SQR = arcs[1].r*arcs[1].r;
    arcs[1].d_SQR = arcs[1].polar_d*arcs[1].polar_d;
    arcs[1].polar_theta = atan2 (v2.y, v2.x);
    arcs[1].integralAtStart = integralOfDistToCircle (a1 - arcs[1].polar_theta, arcs[1].polar_d, arcs[1].r, arcs[1].sign);

    area = CalcAreaToAngle (a2);
  }

  float ScallopedSector::CalcAreaToAngle (float angle)
  {
    float underInner = integralOfDistToCircle (angle - arcs[0].polar_theta, arcs[0].polar_d, arcs[0].r, arcs[0].sign) - arcs[0].integralAtStart;
    float underOuter = integralOfDistToCircle (angle - arcs[1].polar_theta, arcs[1].polar_d, arcs[1].r, arcs[1].sign) - arcs[1].integralAtStart;

    return underOuter - underInner;
  }

  float ScallopedSector::CalcAngleForArea (float area)
  {
    float frnd = frand ();
    if (frnd == 0.f) frnd = FLT_EPSILON;
    float lo = a1, hi = a2, cur = lo + (hi - lo)*frnd;

    for (int i = 0; i < 10; i++) {
      if (CalcAreaToAngle (cur) < area) {
        lo = cur;
        cur = (cur + hi)*.5f;
      }
      else {
        hi = cur;
        cur = (lo + cur)*.5f;
      }
    }

    return cur;
  }

  float ScallopedSector::DistToCurve (float angle, int index)
  {
    float alpha = angle - arcs[index].polar_theta;
    float sin_alpha = sin (alpha);
    float t0 = arcs[index].r_SQR - arcs[index].d_SQR*sin_alpha*sin_alpha;
    if (t0 < 0) {
      return arcs[index].polar_d*cos (alpha);
    }
    else {
      return arcs[index].polar_d*cos (alpha) + arcs[index].sign*sqrt (t0);
    }
  }

  Vector2f ScallopedSector::Sample ()
  {
    float angle = CalcAngleForArea (area*frand());
    float d1 = DistToCurve (angle, 0);
    float d2 = DistToCurve (angle, 1);
    float d = sqrt (d1*d1 + (d2*d2 - d1*d1)*frand());

    return Vector2f (P.x + cos (angle)*d, P.y + sin (angle)*d);
  }

  ///

  float ScallopedSector::CanonizeAngle (float angle)
  {
    float delta = fmod (angle - a1, kTwoPi);
    if (delta < 0) delta += kTwoPi;
    return a1 + delta;
  }

  void ScallopedSector::DistToCircle (float angle, Vector2f &C, float r, float *d1_out, float *d2_out)
  {
    Vector2f v (C.x - P.x, C.y - P.y);
    float dSqrd = v.x*v.x + v.y*v.y;
    float theta = atan2 (v.y, v.x);
    float alpha = angle - theta;
    float sin_alpha = sin (alpha);
    float xSqrd = r*r - dSqrd*sin_alpha*sin_alpha;

    if (xSqrd < 0) {
      *d1_out = *d2_out = -10000000;
    }
    else {
      float a = sqrt (dSqrd)*cos (alpha);
      float x = sqrt (xSqrd);
      *d1_out = a - x;
      *d2_out = a + x;
    }
  }

  void ScallopedSector::SubtractDisk (Vector2f &C, float r, std::vector<ScallopedSector> *regions)
  {
    std::vector<float> angles;

    Vector2f v (C.x - P.x, C.y - P.y);
    float d = sqrt (v.x*v.x + v.y*v.y);

    if (r < d) {
      float theta = atan2 (v.y, v.x);
      float x = sqrt (d*d - r*r);
      float angle, alpha = asin (r / d);

      angle = CanonizeAngle (theta + alpha);
      if (a1 < angle && angle < a2) {
        if (DistToCurve (angle, 0) < x && x < DistToCurve (angle, 1))
          angles.push_back (angle);
      }

      angle = CanonizeAngle (theta - alpha);
      if (a1 < angle && angle < a2) {
        if (DistToCurve (angle, 0) < x && x < DistToCurve (angle, 1))
          angles.push_back (angle);
      }
    }

    for (int arcIndex = 0; arcIndex<2; arcIndex++) {
      Vector2f &C2 = arcs[arcIndex].P;
      float R = arcs[arcIndex].r;
      Vector2f v (C.x - C2.x, C.y - C2.y);
      float d = sqrt (v.x*v.x + v.y*v.y);

      if (d>FLT_EPSILON) {
        float invD = 1.0f / d;
        float x = (d*d - r*r + R*R)*invD*.5f;
        float k = R*R - x*x;

        if (k > 0) {
          float y = sqrt (k);
          float vx = v.x*invD;
          float vy = v.y*invD;
          float vx_x = vx*x, vy_x = vy*x;
          float vx_y = vx*y, vy_y = vy*y;
          float angle;

          angle = CanonizeAngle (atan2 (C2.y + vy_x + vx_y - P.y,
            C2.x + vx_x - vy_y - P.x));
          if (a1 < angle && angle < a2) angles.push_back (angle);

          angle = CanonizeAngle (atan2 (C2.y + vy_x - vx_y - P.y,
            C2.x + vx_x + vy_y - P.x));
          if (a1 < angle && angle < a2) angles.push_back (angle);
        }
      }
    }

    sort (angles.begin (), angles.end ());
    angles.insert (angles.begin (), a1);
    angles.push_back (a2);

    for (unsigned int i = 1; i < angles.size (); i++) {
      float a1 = angles[i - 1], a2 = angles[i];
      float midA = (a1 + a2)*.5f;
      float inner = DistToCurve (midA, 0);
      float outer = DistToCurve (midA, 1);
      float d1, d2;

      DistToCircle (midA, C, r, &d1, &d2); // d1<=d2

      if (d2<inner || d1>outer) {
        regions->push_back (ScallopedSector (P, a1, a2, arcs[0].P, arcs[0].r, arcs[0].sign, arcs[1].P, arcs[1].r, arcs[1].sign));
      }
      else {
        if (inner < d1) {
          regions->push_back (ScallopedSector (P, a1, a2, arcs[0].P, arcs[0].r, arcs[0].sign, C, r, -1));
        }
        if (d2 < outer) {
          regions->push_back (ScallopedSector (P, a1, a2, C, r, 1, arcs[1].P, arcs[1].r, arcs[1].sign));
        }
      }
    }
  }

  ///

  ScallopedRegion::ScallopedRegion (Vector2f P, float r1, float r2, float _minArea) :
    minArea (_minArea)
  {
    m_sectors = new std::vector<ScallopedSector>;
    m_sectors->push_back (ScallopedSector (P, 0, kTwoPi, P, r1, 1, P, r2, 1));
    area = (*m_sectors)[0].area;
  }

  ScallopedRegion::~ScallopedRegion ()
  {
    delete m_sectors;
  }

  bool ScallopedRegion::IsEmpty ()
  {
    return m_sectors->size () == 0;
  }

  void ScallopedRegion::SubtractDisk (Vector2f C, float r)
  {
    std::vector<ScallopedSector> *newRegions = new std::vector<ScallopedSector>;

    area = 0;
    for (unsigned int i = 0; i < m_sectors->size (); i++) {
      ScallopedSector &ss = (*m_sectors)[i];
      std::vector<ScallopedSector> *tmp = new std::vector<ScallopedSector>;

      ss.SubtractDisk (C, r, tmp);

      for (unsigned int j = 0; j<tmp->size (); j++) {
        ScallopedSector &nss = (*tmp)[j];

        if (nss.area>minArea) {
          area += nss.area;

          if (newRegions->size ()) {
            ScallopedSector &last = (*newRegions)[newRegions->size () - 1];
            if (last.a2 == nss.a1 && (last.arcs[0].P == nss.arcs[0].P && last.arcs[0].r == nss.arcs[0].r && last.arcs[0].sign == nss.arcs[0].sign) &&
                (last.arcs[1].P == nss.arcs[1].P && last.arcs[1].r == nss.arcs[1].r && last.arcs[1].sign == nss.arcs[1].sign)) {
              last.a2 = nss.a2;
              last.area = last.CalcAreaToAngle (last.a2);
              continue;
            }
          }

          newRegions->push_back (nss);
        }
      }

      delete tmp;
    }

    delete m_sectors;
    m_sectors = newRegions;
  }

  Vector2f ScallopedRegion::Sample ()
  {
    if (!m_sectors->size ()) {
      printf ("Fatal error, sampled from empty region.");
      exit (1);
      return Vector2f (0, 0);
    }
    else {
      float a = area*frand();
      ScallopedSector &ss = (*m_sectors)[0];

      for (unsigned int i = 0; i < m_sectors->size (); i++) {
        ss = (*m_sectors)[i];
        if (a < ss.area)
          break;
        a -= ss.area;
      }

      return ss.Sample ();
    }
  }
}
