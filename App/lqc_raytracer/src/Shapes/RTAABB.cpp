#include <raytracer/RTRay.h>
#include <raytracer/Shapes/RTAABB.h>

namespace rt
{

  RTAABB::RTAABB ()
    : RTObject (RTObject::OBJECT_TYPE::AABB)
  {}

  RTAABB::RTAABB (lqc::Vector3f _p_min, lqc::Vector3f _p_max, lqc::Vector3f _color)
    : RTObject (RTObject::OBJECT_TYPE::AABB)
  {
    p_min = _p_min;
    p_max = _p_max;
    color = _color;
    setMaterial (RTObject::OBJECT_MATERIAL::DEFAULT);
  }

  RTAABB::~RTAABB ()
  {}

  void RTAABB::Create (lqc::Vector3f _p_min, lqc::Vector3f _p_max, lqc::Vector3f _color)
  {
    p_min = _p_min;
    p_max = _p_max;
    color = _color;
  }

  bool RTAABB::Hit (RTRay ray, float &tmin, lqc::Vector3f &normalv)
  {
    float tmax;
    float x0 = p_min.x; float y0 = p_min.y; float z0 = p_min.z;
    float x1 = p_max.x; float y1 = p_max.y; float z1 = p_max.z;

    float ox = ray.GetPosition ().x;  float oy = ray.GetPosition ().y;  float oz = ray.GetPosition ().z;
    float dx = ray.GetDirection ().x; float dy = ray.GetDirection ().y; float dz = ray.GetDirection ().z;

    float tx_min, ty_min, tz_min;
    float tx_max, ty_max, tz_max;

    float a = 1.0f / dx;
    if (a >= 0.0f)
    {
      tx_min = (x0 - ox) * a;
      tx_max = (x1 - ox) * a;
    }
    else
    {
      tx_min = (x1 - ox) * a;
      tx_max = (x0 - ox) * a;
    }

    float b = 1.0f / dy;
    if (b >= 0.0f)
    {
      ty_min = (y0 - oy) * b;
      ty_max = (y1 - oy) * b;
    }
    else
    {
      ty_min = (y1 - oy) * b;
      ty_max = (y0 - oy) * b;
    }

    float c = 1.0f / dz;
    if (c >= 0.0f)
    {
      tz_min = (z0 - oz) * c;
      tz_max = (z1 - oz) * c;
    }
    else
    {
      tz_min = (z1 - oz) * c;
      tz_max = (z0 - oz) * c;
    }

    float kEpsilon = 0.0f;
    int face_in, face_out;
    float t0, t1;

    //find largest entering t value
    if (tx_min > ty_min){
      t0 = tx_min;
      face_in = (a >= 0.0f) ? 0 : 3;
    }
    else{
      t0 = ty_min;
      face_in = (b >= 0.0f) ? 1 : 4;
    }
    if (tz_min > t0){
      t0 = tz_min;
      face_in = (c >= 0.0f) ? 2 : 5;
    }

    //find smallest exiting t value
    if (tx_max < ty_max){
      t1 = tx_max;
      face_out = (a >= 0.0f) ? 3 : 0;
    }
    else{
      t1 = ty_max;
      face_out = (b >= 0.0f) ? 4 : 1;
    }

    if (tz_max < t1){
      t1 = tz_max;
      face_out = (c >= 0.0f) ? 5 : 2;
    }

    if (t0 < t1 && t1 > kEpsilon)
    {
      if (t0 > kEpsilon)
      {
        tmin = t0;
        normalv = GetNormal (face_in);
        tmax = t1;
      }
      else
      {
        tmin = t1;
        normalv = GetNormal (face_out);
        tmax = t0;
      }
      //hit_point = ray.position + tmin * ray.direction
      return true;
    }
    else
    {
      return false;
    }
  }

  bool RTAABB::Hit (RTRay ray, float &tmin, float &tmax, lqc::Vector3f &normal)
  {
    float x0 = p_min.x; float y0 = p_min.y; float z0 = p_min.z;
    float x1 = p_max.x; float y1 = p_max.y; float z1 = p_max.z;

    float ox = ray.GetPosition ().x;  float oy = ray.GetPosition ().y;  float oz = ray.GetPosition ().z;
    float dx = ray.GetDirection ().x; float dy = ray.GetDirection ().y; float dz = ray.GetDirection ().z;

    float tx_min, ty_min, tz_min;
    float tx_max, ty_max, tz_max;

    float a = 1.0f / dx;
    if (a >= 0.0f)
    {
      tx_min = (x0 - ox) * a;
      tx_max = (x1 - ox) * a;
    }
    else
    {
      tx_min = (x1 - ox) * a;
      tx_max = (x0 - ox) * a;
    }

    float b = 1.0f / dy;
    if (b >= 0.0f)
    {
      ty_min = (y0 - oy) * b;
      ty_max = (y1 - oy) * b;
    }
    else
    {
      ty_min = (y1 - oy) * b;
      ty_max = (y0 - oy) * b;
    }

    float c = 1.0f / dz;
    if (c >= 0.0f)
    {
      tz_min = (z0 - oz) * c;
      tz_max = (z1 - oz) * c;
    }
    else
    {
      tz_min = (z1 - oz) * c;
      tz_max = (z0 - oz) * c;
    }

    float kEpsilon = 0.0f;
    int face_in, face_out;
    float t0, t1;

    //find largest entering t value
    if (tx_min > ty_min){
      t0 = tx_min;
      face_in = (a >= 0.0f) ? 0 : 3;
    }
    else{
      t0 = ty_min;
      face_in = (b >= 0.0f) ? 1 : 4;
    }
    if (tz_min > t0){
      t0 = tz_min;
      face_in = (c >= 0.0f) ? 2 : 5;
    }

    //find smallest exiting t value
    if (tx_max < ty_max){
      t1 = tx_max;
      face_out = (a >= 0.0f) ? 3 : 0;
    }
    else{
      t1 = ty_max;
      face_out = (b >= 0.0f) ? 4 : 1;
    }

    if (tz_max < t1){
      t1 = tz_max;
      face_out = (c >= 0.0f) ? 5 : 2;
    }

    if (t0 < t1 && t1 > kEpsilon)
    {
      if (t0 > kEpsilon)
      {
        tmin = t0;
        normal = GetNormal (face_in);
        tmax = t1;
      }
      else
      {
        tmin = t1;
        normal = GetNormal (face_out);
        tmax = t0;
      }
      //hit_point = ray.position + tmin * ray.direction
      return true;
    }
    else
    {
      return false;
    }
  }

  lqc::Vector3f RTAABB::Normal (lqc::Vector3f point)
  {
    return lqc::Vector3f (0, 0, 0);
  }

  lqc::Vector3f RTAABB::GetNormal (int face_hit)
  {
    switch (face_hit)
    {
      case 0: return lqc::Vector3f (-1.f, 0.f, 0.f);
      case 1: return lqc::Vector3f (0.f, -1.f, 0.f);
      case 2: return lqc::Vector3f (0.f, 0.f, -1.f);
      case 3: return lqc::Vector3f (1.f, 0.f, 0.f);
      case 4: return lqc::Vector3f (0.f, 1.f, 0.f);
      case 5: return lqc::Vector3f (0.f, 0.f, 1.f);
      default: return lqc::Vector3f (0.f, 0.f, 0.f);
    }
  }

  void RTAABB::GetAnchors (lqc::Vector3f &min, lqc::Vector3f &max)
  {
    min = p_min;
    max = p_max;
  }
}