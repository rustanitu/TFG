#ifndef RAYTRACER_TRIANGLE_H
#define RAYTRACER_TRIANGLE_H

#include <raytracer/RTRay.h>
#include <raytracer/Shapes/RTObject.h>
#include <math/Vector3.h>

namespace rt
{
  /***********
         C
        /|
       / |
      /  |
     A---B
  ***********/

  class RTTriangle : public RTObject
  {
  public:
    RTTriangle ();
    RTTriangle (lqc::Vector3f A, lqc::Vector3f B, lqc::Vector3f C,
                lqc::Vector3f icolor = lqc::Vector3f (0.7f, 0.7f, 0.0f));
    ~RTTriangle ();

    void CreateTriangle (lqc::Vector3f A, lqc::Vector3f B, lqc::Vector3f C, 
                         lqc::Vector3f icolor = lqc::Vector3f (0.7f, 0.7f, 0.0f));

    virtual bool Hit (RTRay ray, float &tmin, lqc::Vector3f &normal);
    virtual bool Hit (RTRay ray, float &tmin, lqc::Vector3f &normalv, lqc::Vector3f &localhitpoint);
    virtual lqc::Vector3f Normal (lqc::Vector3f point);

  private:
    lqc::Vector3f m_v0;
    lqc::Vector3f m_v1;
    lqc::Vector3f m_v2;
    lqc::Vector3f m_normal;
  };

}

#endif