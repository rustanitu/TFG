#ifndef RAYTRACER_LIGHT_H
#define RAYTRACER_LIGHT_H

#include <raytracer/RTRay.h>
#include <raytracer/Shapes/RTObject.h>
#include <math/Vector3.h>

namespace rt
{

  class RTLight : public RTObject
  {
  public:
    RTLight ();
    RTLight (lqc::Vector3f position, lqc::Vector3f color = lqc::Vector3f (.8f, .8f, .8f));
    ~RTLight ();

    void lightCreate (lqc::Vector3f position, lqc::Vector3f color = lqc::Vector3f (.8f, .8f, .8f));

    virtual bool Hit (RTRay ray, float &tmin, lqc::Vector3f &normal);
    virtual lqc::Vector3f Normal (lqc::Vector3f point);

    lqc::Vector3f position;
  private:
    float radius, slices, stacks;
  };

}

#endif