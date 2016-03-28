#ifndef RAYTRACER_RECTANGLE_H
#define RAYTRACER_RECTANGLE_H

#include <raytracer/RTRay.h>
#include <raytracer/Shapes/RTObject.h>
#include <math/Vector3.h>

namespace rt
{
  class RTRectangle : public RTObject
  {
  public:
    RTRectangle ();
    RTRectangle (lqc::Vector3f A, lqc::Vector3f B, 
                 lqc::Vector3f C, lqc::Vector3f icolor = lqc::Vector3f (0.7f, 0.7f, 0.0f));
    ~RTRectangle ();

    void CreateRectangle (lqc::Vector3f A, lqc::Vector3f B,
                          lqc::Vector3f C, lqc::Vector3f icolor = lqc::Vector3f (0.7f, 0.7f, 0.0f));

    virtual bool Hit (RTRay ray, float &tmin, lqc::Vector3f &normal);
    virtual bool Hit (RTRay ray, float &tmin, lqc::Vector3f &normalv, lqc::Vector3f &localhitpoint);
    virtual void draw ();
    virtual lqc::Vector3f normal (lqc::Vector3f point);

  private:
    lqc::Vector3f m_p0;
    lqc::Vector3f m_normal;
    lqc::Vector3f m_a;
    lqc::Vector3f m_b;
  };
}

#endif