#include <raytracer/Shapes/RTObject.h>
#include <math/Vector3.h>

namespace rt
{

  RTObject::RTObject ()
  {}

  RTObject::RTObject (OBJECT_TYPE obj_t)
  {
    type = obj_t;
  }

  RTObject::~RTObject ()
  {}

  bool RTObject::Hit (RTRay ray, float &tmin, lqc::Vector3f &normal, lqc::Vector3f &localhitpoint)
  {
    return false;
  }

  bool RTObject::Hit (RTRay ray, float &tmin, float &tmax, lqc::Vector3f &normal)
  {
    return false;
  }

  void RTObject::Draw ()
  {}

  RTObject::OBJECT_TYPE RTObject::getType ()
  {
    return type;
  }

  void RTObject::setType (RTObject::OBJECT_TYPE _type)
  {
    type = _type;
  }

  RTObject::OBJECT_MATERIAL RTObject::getMaterial ()
  {
    return material;
  }

  void RTObject::setMaterial (RTObject::OBJECT_MATERIAL _material)
  {
    material = _material;
  }

}