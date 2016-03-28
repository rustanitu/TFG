#ifndef RAYTRACER_OBJECT_H
#define RAYTRACER_OBJECT_H

#include <raytracer/RTRay.h>
#include <math/Vector3.h>

namespace rt
{

  class RTObject
  {
  public:
    enum OBJECT_TYPE
    {
      AABB,
      BOX,
      DISK,
      LIGHT,
      RECTANGLE,
      SPHERE,
      TRIANGLE,
      VOLUME,
    };

    enum OBJECT_MATERIAL
    {
      DEFAULT,
      REFLECTIVE,
      SIMPLE_TRANSPARENCY,
      REAL_TRANSPARENCY
    };

    RTObject ();
    RTObject (OBJECT_TYPE obj_type);
    ~RTObject ();

    virtual bool Hit (RTRay ray, float &tmin, lqc::Vector3f &normal) = 0;
    virtual bool Hit (RTRay ray, float &tmin, lqc::Vector3f &normal, lqc::Vector3f &localhitpoint);
    virtual bool Hit (RTRay ray, float &tmin, float &tmax, lqc::Vector3f &normal);

    virtual void Draw ();
    virtual lqc::Vector3f Normal (lqc::Vector3f point) = 0;

    OBJECT_TYPE getType ();
    OBJECT_MATERIAL getMaterial ();
    void setType (RTObject::OBJECT_TYPE type);
    void setMaterial (RTObject::OBJECT_MATERIAL material);

    lqc::Vector3f color;
  private:
    OBJECT_TYPE type;
    OBJECT_MATERIAL material;
  };

}

#endif