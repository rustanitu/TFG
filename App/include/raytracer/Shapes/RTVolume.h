#ifndef RAYTRACER_VOLUME_H
#define RAYTRACER_VOLUME_H

#include <raytracer/RTRay.h>
#include <raytracer/Shapes/RTObject.h>
#include <raytracer/Shapes/RTAABB.h>

#include <math/Vector3.h>

namespace rt
{
  class RTVolume : public RTObject
  {
  public:
    RTVolume ();
    RTVolume (lqc::Vector3f p_min, lqc::Vector3f p_max);
    ~RTVolume ();

    void Create (lqc::Vector3f p_min, lqc::Vector3f p_max, const char* filename, size_t bytes_per_value,
                 unsigned int vol_width, unsigned int vol_height, unsigned int vol_depth);

    virtual bool Hit (RTRay ray, float &tmin, lqc::Vector3f &normal);
    virtual bool Hit (RTRay ray, float &tmin, lqc::Vector3f &normalv, lqc::Vector3f &localhitpoint);
    virtual bool Hit (RTRay ray, float &tmin, float &tmax, lqc::Vector3f &normal);
    virtual lqc::Vector3f Normal (lqc::Vector3f point);

  private:
    RTAABB m_rt_box;
  };

}

#endif