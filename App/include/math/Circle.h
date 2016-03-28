#ifndef LQC_MATH_CIRCLE_H
#define LQC_MATH_CIRCLE_H

#include <math/Vector2.h>

namespace lqc
{
  typedef struct Circle2f
  {
    Circle2f ()
      : center(Vector2f(0.0f,0.0f)), radius(0)
    {}

    Circle2f (Vector2f m_center, float m_radius)
      : center(m_center), radius(m_radius)
    {}

    Circle2f& operator=(const Circle2f& c)  
    {  
      this->center = c.center;
      this->radius = c.radius;

      return *this;
    }

    std::vector<Vector2f> convertToPolygon (int n_points, bool is_counter_clockwise = true)
    {
      std::vector<Vector2f> polygon;
      double iteration = (2.0*3.1415926535897932384626433832795028841971693993751058)/(double)n_points;
      double angle = 0.0f;
      if (is_counter_clockwise)
      {
        for(int i = 0 ; i < n_points ; i++)
        {
          polygon.push_back(Vector2f(center.x + radius*(float)cos(angle), center.y + radius*(float)sin(angle)));
          angle += iteration;
        }
      }
      else
      {
        for(int i = 0 ; i < n_points ; i++)
        {
          polygon.push_back(Vector2f(center.x + radius*(float)cos(angle), center.y + radius*(float)-sin(angle)));
          angle += iteration;
        }
      }
      return polygon;
    }

    float radius;
    lqc::Vector2f center;
  } Circle2f; 

}

#endif