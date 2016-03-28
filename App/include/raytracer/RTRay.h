#ifndef RAYTRACER_RAY_H
#define RAYTRACER_RAY_H

#include <math/Vector3.h>

namespace rt
{

  class RTRay
  {
  public:
    RTRay (lqc::Vector3f position, lqc::Vector3f direction);
    ~RTRay ();

    lqc::Vector3f GetPosition ();
    lqc::Vector3f GetDirection ();

  private:
    lqc::Vector3f m_position;
    lqc::Vector3f m_direction;
  };

}

#endif