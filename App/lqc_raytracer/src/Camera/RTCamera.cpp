#include <raytracer/Camera/RTCamera.h>
#include <lqc/lqcdefines.h>
#include <math/MGeometry.h>
#include <math/MUtils.h>
#include <math/Vector4.h>
#include <math/Matrix4.h>
#include <math/Quaternion.h>

#include <glm/glm.hpp>

namespace rt
{

  RTCamera::RTCamera ()
    : m_width (0), m_height (0)//Camera (0, 0)
    , m_projection (CamProjection::PERSPECTIVE)
  {
    m_mov_speed = 5.0f;
    m_rot_speed = 2.0f;

    m_r_b = 100.0f;
    m_r_h = 100.0f;
  }

  RTCamera::RTCamera (RTCamera::CamProjection proj)
    : m_width (0), m_height (0)//Camera (0, 0)
    , m_projection (proj)
  {
    m_mov_speed = 5.0f;
    m_rot_speed = 2.0f;

    m_r_b = 100.0f;
    m_r_h = 100.0f;
  }

  RTCamera::~RTCamera ()
  {
  }

  void RTCamera::Init (void *data)
  {
    Struct_RTCamera_init* st = (Struct_RTCamera_init*)data;
   
    m_eye = st->eye;
    m_at = st->at;
    m_up = st->up;

    m_init_eye = m_eye;
    m_init_center = m_at;
    m_init_iup = m_up;

    m_fov_y = st->fovy;
    m_near_plane = st->nearp;
    m_far_plane = st->farp;

    m_width = st->w;
    m_height = st->h;
    m_r_b = m_width / 16.f;
    m_r_h = m_height / 16.f;
    aspect = m_width / m_height;

    f = m_near_plane;
    a = 2.0f * f * (float)tan (((m_fov_y / 2.0f) / 180.f)*PI);
    b = (m_width / m_height) * a;

    m_z_e = m_eye - m_at;
    m_z_e = lqc::Vector3f::Normalize (m_z_e);
    m_x_e = CrossProduct (m_up, m_z_e);
    m_x_e = lqc::Vector3f::Normalize (m_x_e);
    m_y_e = CrossProduct (m_z_e, m_x_e);
    m_y_e = lqc::Vector3f::Normalize (m_y_e);

    // printf ("%f %f %f\n %f %f %f\n %f %f %f\n", m_z_e.x, m_z_e.y, m_z_e.z, m_y_e.x, m_y_e.y, m_y_e.z, m_x_e.x, m_x_e.y, m_x_e.z);
  }

  void RTCamera::Reset ()
  {
    Struct_RTCamera_init st;
    st.eye = m_init_eye;
    st.at = m_init_center;
    st.up = m_init_iup;

    st.fovy = m_fov_y;
    st.nearp = m_near_plane;
    st.farp = m_far_plane;
    st.w = m_width;
    st.h = m_height;
    
    Init (&st);
  }

  void RTCamera::Keyboard_cb (unsigned char key, int x, int y)
  {
    switch (key) {
      case 'w':
        m_eye -= m_z_e * m_mov_speed;
        break;
      case 's':
        m_eye += m_z_e * m_mov_speed;
        break;
      case 'a':
        m_eye -= m_x_e * m_mov_speed;
        break;
      case 'd':
        m_eye += m_x_e * m_mov_speed;
        break;
      case 'e':
        m_eye += m_y_e * m_mov_speed;
        break;
      case 'q':
        m_eye -= m_y_e * m_mov_speed;
        break;
      default:
        break;
    }
    //printf ("eye %f %f %f\n", eye.x, eye.y, eye.z);
  }

  lqc::Vector3f RTCamera::GetPositionCastRay (float x, float y)
  {
    if (m_projection == CamProjection::PERSPECTIVE)
      return GetPosition ();
    else if (m_projection == CamProjection::ORTHOGRAPHIC)
    {
      lqc::Vector3f p = GetPosition ();
      p = p + (
        ((((y / m_height) - (1.f / 2.f)) * 2.0f) * m_r_h)
        * m_y_e);
      p = p + (
        ((((x / m_width) - (1.f / 2.f)) * 2.0f) * m_r_b)
               * m_x_e);
      return p;
    }
    return lqc::Vector3f::Zero ();
  }

  lqc::Vector3f RTCamera::CastRay (float x, float y)
  {
    if (m_projection == CamProjection::PERSPECTIVE)
    {
      lqc::Vector3f d =
        (m_z_e * f) * -1.0f
        + (m_y_e * (a * ((y / m_height) - (1.0f / 2.0f))))
        + (m_x_e * (b * ((x / m_width) - (1.0f / 2.0f))));
      d = lqc::Vector3f::Normalize (d);
      return d;
    }
    else if (m_projection == CamProjection::ORTHOGRAPHIC)
    {
      lqc::Vector3f d = -1.0f * m_z_e;
      d = lqc::Vector3f::Normalize (d);
      return d;
    }
    else
      return lqc::Vector3f::Zero();
  }

  void RTCamera::MoveForward (float speed)
  {
    m_eye += m_z_e * speed;
  }

  void RTCamera::MoveUp (float speed)
  {
    m_eye += m_y_e * speed;
  }

  void RTCamera::MoveRight (float speed)
  {
    m_eye += m_x_e * speed;
  }

  void RTCamera::RotateX (float angle)
  {
    RotateCamera (angle, m_x_e);
  }

  void RTCamera::RotateY (float angle)
  {
    RotateCamera (angle, m_y_e);
  }

  void RTCamera::RotateZ (float angle)
  {
    RotateCamera (angle, m_z_e);
  }

  void RTCamera::RotateCamera (double Angle, double x, double y, double z)
  {
    Angle = Angle * m_rot_speed;
    //printf ("m_z_e = %f, %f, %f\n", m_z_e.x, m_z_e.y, m_z_e.z);
    //printf ("m_x_e = %f, %f, %f\n", m_x_e.x, m_x_e.y, m_x_e.z);
    //printf ("m_y_e = %f, %f, %f\n", m_y_e.x, m_y_e.y, m_y_e.z);

    lqc::Quaternion temp, quat_view, result;

    temp.x = x * sin (Angle / 2.0f);
    temp.y = y * sin (Angle / 2.0f);
    temp.z = z * sin (Angle / 2.0f);
    temp.w = cos (Angle / 2.0f);

    quat_view.x = m_z_e.x;
    quat_view.y = m_z_e.y;
    quat_view.z = m_z_e.z;
    quat_view.w = 0;

    result = lqc::Quaternion::Multiply
      (lqc::Quaternion::Multiply (temp, quat_view),
      lqc::Quaternion::Conjugate (temp));

    m_z_e.x = result.x;
    m_z_e.y = result.y;
    m_z_e.z = result.z;

    m_z_e = lqc::Vector3f::Normalize (m_z_e);
    m_x_e = CrossProduct (m_y_e, m_z_e);
    m_x_e = lqc::Vector3f::Normalize (m_x_e);
    m_y_e = CrossProduct (m_z_e, m_x_e);
    m_y_e = lqc::Vector3f::Normalize (m_y_e);

    //printf ("m_z_e = %f, %f, %f\n", m_z_e.x, m_z_e.y, m_z_e.z);
    //printf ("m_x_e = %f, %f, %f\n", m_x_e.x, m_x_e.y, m_x_e.z);
    //printf ("m_y_e = %f, %f, %f\n", m_y_e.x, m_y_e.y, m_y_e.z);

  }

  void RTCamera::RotateCamera (double Angle, lqc::Vector3f v)
  {
    RotateCamera (Angle, v.x, v.y, v.z);
  }

  void RTCamera::UseOrthographic (float rec_base, float rec_height)
  {
    m_r_h = rec_height;
    m_r_b = rec_base;
  }

  void RTCamera::UseOrthographic (float div)
  {
    m_r_h = m_height / div;
    m_r_b = m_width / div;
  }

}
