#include <raytracer/Shapes/RTSphere.h>
#include <math/MGeometry.h>

namespace rt
{
  RTSphere::RTSphere ()
    : RTObject (RTObject::OBJECT_TYPE::SPHERE)
  {
    slices = 50;
    stacks = 50;
  }

  RTSphere::RTSphere (lqc::Vector3f _position, float _radius, lqc::Vector3f _color)
    : RTObject (RTObject::OBJECT_TYPE::SPHERE)
  {
    center = _position;
    radius = _radius;
    color = _color;
    slices = 50;
    stacks = 50;
  }

  RTSphere::~RTSphere ()
  {
  }

  void RTSphere::sphereCreate (lqc::Vector3f _position, float _radius, lqc::Vector3f _color)
  {
    center = _position;
    radius = _radius;
    color = _color;
    slices = 50;
    stacks = 50;
  }

  bool RTSphere::Hit (RTRay ray, float &tmin, lqc::Vector3f &normalv)
  {
    float a = Dot (ray.GetDirection (), ray.GetDirection ());
    float b = Dot (2.f * ray.GetDirection (), ray.GetPosition () - center);
    float c = Dot (ray.GetPosition () - center, ray.GetPosition () - center) - (radius * radius);

    float delta = b * b - 4.0f * a * c;
    if (delta > 0.0f)
    {
      float t1 = (-b - sqrt (delta)) / (2.f * a);
      float t2 = (-b + sqrt (delta)) / (2.f * a);
      tmin = std::min (t1, t2);
      normalv = Normal (ray.GetPosition () + tmin * ray.GetDirection ());
      return true;
    }
    return false;
  }

  lqc::Vector3f RTSphere::Normal (lqc::Vector3f point)
  {
    return lqc::Vector3f::Normalize (point - center);
  }

}