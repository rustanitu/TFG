#include <raytracer/RTRay.h>
#include <math/Vector3.h>

namespace rt
{

  RTRay::RTRay (lqc::Vector3f position, lqc::Vector3f direction)
  {
    m_position = position;
    m_direction = direction;
  }

  RTRay::~RTRay ()
  {}

  lqc::Vector3f RTRay::GetPosition ()
  {
    return m_position;
  }

  lqc::Vector3f RTRay::GetDirection ()
  {
    return m_direction;
  }

}