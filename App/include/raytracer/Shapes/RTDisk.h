#ifndef RAYTRACER_DISK_H
#define RAYTRACER_DISK_H

#include <raytracer/RTRay.h>
#include <raytracer/Shapes/RTObject.h>
#include <math/Vector3.h>

namespace rt
{

  class RTDisk : public RTObject
  {
  public:
    RTDisk ();
    RTDisk (lqc::Vector3f center, float radius, 
            lqc::Vector3f normal, lqc::Vector3f icolor = lqc::Vector3f (0.7f, 0.7f, 0.0f));
    ~RTDisk ();

    void CreateDisk (lqc::Vector3f center, float radius, lqc::Vector3f normal, lqc::Vector3f icolor = lqc::Vector3f (0.7f, 0.7f, 0.0f));

    virtual bool Hit (RTRay ray, float &tmin, lqc::Vector3f &normal);
    virtual bool Hit (RTRay ray, float &tmin, lqc::Vector3f &normalv, lqc::Vector3f &localhitpoint);
    virtual lqc::Vector3f normal (lqc::Vector3f point);

  private:
    lqc::Vector3f m_center;
    float m_radius;
    lqc::Vector3f m_normal;
  };

}

#endif