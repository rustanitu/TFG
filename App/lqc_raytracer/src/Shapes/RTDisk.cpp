#include <raytracer/RTRay.h>
#include <raytracer/Shapes/RTDisk.h>
#include <math/MGeometry.h>
#include <lqc/lqcdefines.h>
#include <math/Vector3.h>

namespace rt
{

  RTDisk::RTDisk ()
    : RTObject (RTObject::OBJECT_TYPE::DISK)
  {
    m_center = lqc::Vector3f (0, 0, 0);
    m_radius = 5;
    m_normal = lqc::Vector3f (0, 1, 0);
    color = lqc::Vector3f (0.7f, 0.7f, 0.0f);
  }

  RTDisk::RTDisk (lqc::Vector3f center, float radius, lqc::Vector3f normal, lqc::Vector3f icolor)
    : RTObject (RTObject::OBJECT_TYPE::DISK)
  {
    CreateDisk (center, radius, normal, icolor);
  }

  RTDisk::~RTDisk ()
  {}

  void RTDisk::CreateDisk (lqc::Vector3f center, float radius, lqc::Vector3f normal, lqc::Vector3f icolor)
  {
    m_center = lqc::Vector3f (center.x, center.y, center.z);
    m_radius = radius;
    m_normal = lqc::Vector3f (normal.x, normal.y, normal.z);
    color = lqc::Vector3f (0.7f, 0.7f, 0.0f);
  }

  bool RTDisk::Hit (RTRay ray, float &tmin, lqc::Vector3f &normalv)
  {
    lqc::Vector3f center = lqc::Vector3f (m_center.x, m_center.y, m_center.z);
    lqc::Vector3f normal = lqc::Vector3f (m_normal.x, m_normal.y, m_normal.z);

    float t = Dot ((center - ray.GetPosition ()), normal) / Dot (ray.GetDirection (), normal);

    if (t <= KEPSILON)
      return false;

    lqc::Vector3f p = ray.GetPosition () + ray.GetDirection () * t;

    float r_squared = m_radius*m_radius;
    float distance = Distance (center, p);
    float d_squared = distance*distance;
    if (d_squared < r_squared)
    {
      tmin = t;
      normalv = normal;
      return true;
    }
    else
      return false;
  }

  bool RTDisk::Hit (RTRay ray, float &tmin, lqc::Vector3f &normalv, lqc::Vector3f &localhitpoint)
  {
    if (!Hit (ray, tmin, normalv))
      return false;
    localhitpoint = ray.GetPosition () + ray.GetDirection () * tmin;
    return true;
  }

  lqc::Vector3f RTDisk::normal (lqc::Vector3f point)
  {
    return lqc::Vector3f (0, 0, 0);
  }

}