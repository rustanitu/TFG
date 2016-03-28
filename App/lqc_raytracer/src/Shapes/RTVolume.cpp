#include <raytracer/RTRay.h>
#include <raytracer/Shapes/RTVolume.h>
#include <math/MGeometry.h>
#include <math/Vector3.h>
#include <lqc/lqcdefines.h>

namespace rt
{

  RTVolume::RTVolume ()
    : RTObject (RTObject::OBJECT_TYPE::VOLUME)
  {
    lqc::Vector3f min (0.0f), max (1.0f);
    m_rt_box.Create (min, max);
  }

  RTVolume::RTVolume (lqc::Vector3f p_min, lqc::Vector3f p_max)
    : RTObject (RTObject::OBJECT_TYPE::VOLUME)
  {
    m_rt_box.Create (p_min, p_max);
  }

  RTVolume::~RTVolume ()
  {}

  void RTVolume::Create (lqc::Vector3f p_min, lqc::Vector3f p_max, const char* filename, size_t bytes_per_value,
    unsigned int vol_width, unsigned int vol_height, unsigned int vol_depth)
  {
    /*m_rt_box.Create (p_min, p_max);
    m_ut_vol.ReadVolume (filename, bytes_per_value, vol_width, vol_height, vol_depth);
    m_ut_vol.SetAnchors (p_min, p_max);*/
  }

  bool RTVolume::Hit (RTRay ray, float &tmin, lqc::Vector3f &normalv)
  {
    return m_rt_box.Hit (ray, tmin, normalv);
  }

  bool RTVolume::Hit (RTRay ray, float &tmin, lqc::Vector3f &normalv, lqc::Vector3f &localhitpoint)
  {
    if (!m_rt_box.Hit (ray, tmin, normalv))
      return false;
    localhitpoint = ray.GetPosition () + ray.GetDirection () * tmin;
    return true;
  }

  bool RTVolume::Hit (RTRay ray, float &tmin, float &tmax, lqc::Vector3f &normal)
  {
    return m_rt_box.Hit (ray, tmin, tmax, normal);
  }

  lqc::Vector3f RTVolume::Normal (lqc::Vector3f point)
  {
    return m_rt_box.Normal (point);
  }

}