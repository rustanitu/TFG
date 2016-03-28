#include <raytracer/RTRay.h>
#include <raytracer/Shapes/RTRectangle.h>
#include <math/MGeometry.h>
#include <math/Vector3.h>
#include <lqc/lqcdefines.h>

namespace rt
{

  RTRectangle::RTRectangle ()
    : RTObject (RTObject::OBJECT_TYPE::RECTANGLE)
  {

  }

  RTRectangle::RTRectangle (lqc::Vector3f A, lqc::Vector3f B,
                            lqc::Vector3f C, lqc::Vector3f icolor)
    : RTObject (RTObject::OBJECT_TYPE::RECTANGLE)
  {
    CreateRectangle (A, B, C, icolor);
  }

  RTRectangle::~RTRectangle ()
  {}

  void RTRectangle::CreateRectangle (lqc::Vector3f A, lqc::Vector3f B,
                                     lqc::Vector3f C, lqc::Vector3f icolor)
  {
  }

  bool RTRectangle::Hit (RTRay ray, float &tmin, lqc::Vector3f &normalv)
  {
    lqc::Vector3f p0 = lqc::Vector3f (m_p0.x, m_p0.y, m_p0.z);
    lqc::Vector3f normal = lqc::Vector3f (m_normal.x, m_normal.y, m_normal.z);
    lqc::Vector3f a = lqc::Vector3f (m_a.x, m_a.y, m_a.z);
    lqc::Vector3f b = lqc::Vector3f (m_b.x, m_b.y, m_b.z);

    float t = Dot (p0 - ray.GetPosition (), normal) / Dot (ray.GetDirection (), normal);

    if (t <= KEPSILON)
      return false;

    lqc::Vector3f p = ray.GetPosition () + ray.GetDirection () * t;
    lqc::Vector3f d = p - p0;

    float ddota = Dot (d, a);

    //TODO
    float a_len_squared = 0.0f;

    if (ddota < 0.0 || ddota > a_len_squared)
      return false;

    float ddotb = Dot (d, b);

    //TODO
    float b_len_squared = 0.0f;

    if (ddotb < 0.0 || ddotb > b_len_squared)
      return false;

    tmin = t;
    normalv = normal;

    return true;
  }

  bool RTRectangle::Hit (RTRay ray, float &tmin, lqc::Vector3f &normalv, 
                         lqc::Vector3f &localhitpoint)
  {
    if (!Hit (ray, tmin, normalv))
      return false;
    localhitpoint = ray.GetPosition () + ray.GetDirection () * tmin;
    return true;
  }

  void RTRectangle::draw ()
  {
  }

  lqc::Vector3f RTRectangle::normal (lqc::Vector3f point)
  {
    return lqc::Vector3f (0, 0, 0);
  }

}