#ifndef RAYTRACER_SPHERE_H
#define RAYTRACER_SPHERE_H

#include <raytracer/RTRay.h>
#include <raytracer/Shapes/RTObject.h>
#include <math/Vector3.h>

namespace rt
{

  class RTSphere : public RTObject
  {
  public:
    RTSphere ();
    RTSphere (lqc::Vector3f position, float radius, 
              lqc::Vector3f color = lqc::Vector3f (0.f, 0.f, 1.f));
    ~RTSphere ();

    void sphereCreate (lqc::Vector3f position, float radius, 
                       lqc::Vector3f color = lqc::Vector3f (0.f, 0.f, 1.f));

    virtual bool Hit (RTRay ray, float &tmin, lqc::Vector3f &normal);
    virtual lqc::Vector3f Normal (lqc::Vector3f point);

  private:
    lqc::Vector3f center;
    float radius;
    int slices, stacks;
  };

}

#endif