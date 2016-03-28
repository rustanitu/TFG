#ifndef RAYTRACER_AABB_H
#define RAYTRACER_AABB_H

#include <raytracer/RTRay.h>
#include <raytracer/Shapes/RTObject.h>
#include <math/Vector3.h>

namespace rt
{
  class RTAABB : public RTObject
  {
  public:
    RTAABB ();
    RTAABB (lqc::Vector3f p_min, lqc::Vector3f p_max, 
            lqc::Vector3f color = lqc::Vector3f (0.7f, 0.7f, 0.0f));
    ~RTAABB ();

    void Create (lqc::Vector3f p_min, lqc::Vector3f p_max, 
                 lqc::Vector3f color = lqc::Vector3f (0.7f, 0.7f, 0.0f));

    virtual bool Hit (RTRay ray, float &tmin, lqc::Vector3f &normal);
    virtual bool Hit (RTRay ray, float &tmin, float &tmax, lqc::Vector3f &normal);
    virtual lqc::Vector3f Normal (lqc::Vector3f point);
    lqc::Vector3f GetNormal (int face_hit);

    void GetAnchors (lqc::Vector3f &min, lqc::Vector3f &max);
  
  private:
    lqc::Vector3f p_min, p_max;
  };
}

#endif

