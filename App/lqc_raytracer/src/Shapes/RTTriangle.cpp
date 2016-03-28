#include <raytracer/RTRay.h>
#include <raytracer/Shapes/RTTriangle.h>
#include <math/MGeometry.h>
#include <math/Vector3.h>
#include <lqc/lqcdefines.h>

namespace rt
{
  RTTriangle::RTTriangle ()
    : RTObject (RTObject::OBJECT_TYPE::TRIANGLE)
  {
    m_v0 = lqc::Vector3f (0, 0, 0);
    m_v1 = lqc::Vector3f (0, 0, 1);
    m_v2 = lqc::Vector3f (1, 0, 0);
    m_normal = lqc::Vector3f (0, 1, 0);
    color = lqc::Vector3f (0.7f, 0.7f, 0.0f);
  }

  RTTriangle::RTTriangle (lqc::Vector3f A, lqc::Vector3f B,
                          lqc::Vector3f C, lqc::Vector3f icolor)
    : RTObject (RTObject::OBJECT_TYPE::TRIANGLE)
  {
    CreateTriangle (A, B, C, icolor);
  }

  RTTriangle::~RTTriangle ()
  {}

  void RTTriangle::CreateTriangle (lqc::Vector3f A, lqc::Vector3f B,
                                   lqc::Vector3f C, lqc::Vector3f icolor)
  {
    m_v0 = lqc::Vector3f (A.x, A.y, A.z);
    m_v1 = lqc::Vector3f (B.x, B.y, B.z);
    m_v2 = lqc::Vector3f (C.x, C.y, C.z);
    m_normal = Cross (m_v1 - m_v0, m_v2 - m_v0);
    m_normal.Normalize ();
    color = icolor;
  }

  bool RTTriangle::Hit (RTRay ray, float &tmin, lqc::Vector3f &normalv)
  {
    lqc::Vector3f v0 = m_v0;
    lqc::Vector3f v1 = m_v1;
    lqc::Vector3f v2 = m_v2;

    double a = v0.x - v1.x, b = v0.x - v2.x, c = ray.GetDirection ().x, d = v0.x - ray.GetPosition ().x;
    double e = v0.y - v1.y, f = v0.y - v2.y, g = ray.GetDirection ().y, h = v0.y - ray.GetPosition ().y;
    double i = v0.z - v1.z, j = v0.z - v2.z, k = ray.GetDirection ().z, l = v0.z - ray.GetPosition ().z;

    double m = f * k - g * j, n = h * k - g * l, p = f * l - h * j;
    double q = g * i - e * k, s = e * j - f * i;

    double inv_denom = 1.0 / (a * m + b * q + c * s);

    double e1 = d * m - b * n - c * p;
    double beta = e1 * inv_denom;

    if (beta < 0.0)
      return false;

    double r = e * l - h * i;
    double e2 = a * n + d * q + c * r;
    double gama = e2 * inv_denom;

    if (gama < 0.0)
      return false;

    if (beta + gama > 1.0)
      return false;

    double e3 = a * p - b * r + d * s;
    double t = e3 * inv_denom;

    if (t < KEPSILON)
      return false;

    tmin = (float)t;
    normalv = lqc::Vector3f (m_normal.x, m_normal.y, m_normal.z);

    return true;
  }

  bool RTTriangle::Hit (RTRay ray, float &tmin, lqc::Vector3f &normalv, 
                        lqc::Vector3f &localhitpoint)
  {
    if (!Hit (ray, tmin, normalv))
      return false;
    localhitpoint = ray.GetPosition () + ray.GetDirection () * tmin;
    return true;
  }

  lqc::Vector3f RTTriangle::Normal (lqc::Vector3f point)
  {
    return m_normal;
  }

}