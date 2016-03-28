#include <lqc/Utils/Camera.h>
#include <math/Matrix.h>
#include <lqc/lqcdefines.h>

namespace lqc
{
  /*Camera::Camera ()
  {
    m_speed = 0.1f;
    m_rot_sensivity = 0.1f;

    m_p = Vector3f (0.0f);

    m_f = Vector3f (0.0f, 0.0f, -1.0f);
    m_u = Vector3f (0.0f, 1.0f, 0.0f);
    m_r = Vector3f (1.0f, 0.0f, 0.0f);

    m_total_rotate = Vector3f (0);
  }

  Camera::~Camera ()
  {
  }

  void Camera::Move (float forward, float up, float right)
  {
    m_p = m_p + (m_f*forward) + (m_u*up) + (m_r*right);
  }

  void Camera::Rotate (float axis_x, float axis_y)
  {
    m_total_rotate.x += axis_x;
    m_total_rotate.y += axis_x;
    
    float ax = axis_x * DEGREE_TO_RADS;
    float ay = axis_y * DEGREE_TO_RADS;
    ax = ax * m_rot_sensivity;
    ay = ay * m_rot_sensivity;
     
    Matrix3f rot_x;
    rot_x.Zero ();
    rot_x.m[0] = 1;
    rot_x.m[4] = cos(ax);
    rot_x.m[5] = -sin (ax);
    rot_x.m[7] = sin (ax);
    rot_x.m[8] = cos (ax);

    Matrix3f rot_y;
    rot_y.Zero ();
    rot_y.m[0] = cos(ay);
    rot_x.m[2] = sin(ay);
    rot_x.m[4] = 1;
    rot_x.m[6] = -sin (ay);
    rot_x.m[8] = cos (ay);

    //rotate by X
    m_f = rot_x * m_f;
    m_u = rot_x * m_u;
    m_r = rot_x * m_r;

    //rotate by y
    m_f = rot_y * m_f;
    m_u = rot_y * m_u;
    m_r = rot_y * m_r;

  }*/
}