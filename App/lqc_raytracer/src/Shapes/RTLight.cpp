#include <raytracer/RTRay.h>
#include <raytracer/Shapes/RTLight.h>
#include <math/Vector3.h>

namespace rt
{
  RTLight::RTLight ()
    : RTObject (RTObject::OBJECT_TYPE::LIGHT)
  {
    radius = 10;
    slices = 20;
    stacks = 20;
  }


  RTLight::RTLight (lqc::Vector3f _position, lqc::Vector3f _color)
    : RTObject (RTObject::OBJECT_TYPE::LIGHT)
  {
    position = _position;
    color = _color;
    radius = 10;
    slices = 20;
    stacks = 20;
  }


  RTLight::~RTLight ()
  {}

  void RTLight::lightCreate (lqc::Vector3f _position, lqc::Vector3f _color)
  {
    position = _position;
    color = _color;
  }

  bool RTLight::Hit (RTRay ray, float &tmin, lqc::Vector3f &normal)
  {
    return false;

  }

  lqc::Vector3f RTLight::Normal (lqc::Vector3f point)
  {
    return lqc::Vector3f::Normalize (position - point);
  }

}