#ifndef RAYTRACER_CAMERA_H
#define RAYTRACER_CAMERA_H

#include <math/Camera.h>

#include <math/Vector3.h>
#include <math/Quaternion.h>

namespace rt
{
  typedef struct Struct_RTCamera_init
  {
    lqc::Vector3f eye;
    lqc::Vector3f at;
    lqc::Vector3f up;

    float fovy;
    float nearp;
    float farp;
    float w;
    float h;
  };

  class RTCamera //: public lqc::Camera
  {
  public:
    enum CamProjection
    {
      ORTHOGRAPHIC,
      PERSPECTIVE,
    };

  public:
    virtual const char* GetNameClass () { return "RTCamera"; }

    virtual void Init (void* data = NULL);
    virtual void Reset ();

    virtual void Keyboard_cb (unsigned char key, int x, int y);

    virtual lqc::Vector3f GetPositionCastRay (float x, float y);
    virtual lqc::Vector3f CastRay (float x, float y);

  public:
    RTCamera ();
    RTCamera (RTCamera::CamProjection);
    ~RTCamera ();

    void MoveForward (float speed);
    void MoveUp (float speed);
    void MoveRight (float speed);

    void RotateX (float angle);
    void RotateY (float angle);
    void RotateZ (float angle);

    void RotateCamera (double Angle, double x, double y, double z);
    void RotateCamera (double Angle, lqc::Vector3f v);

    void UseOrthographic (float rec_base, float rec_height);
    void UseOrthographic (float div);

    ////TEMPORARIO
    lqc::Vector3f GetPosition () { return m_eye; }

    void SetMovementSpeed (float sp) { m_mov_speed = sp; }
    float GetMovementSpeed () { return m_mov_speed; }

    void SetRotationSpeed (float sp) { m_rot_speed = sp; }
    float GetRotationSpeed () { return m_rot_speed; }
    ////TEMPORARIO

    float GetNearPlane ()
    {
      return m_near_plane;
    }

    float GetFarPlane ()
    {
      return m_far_plane;
    }

    lqc::Vector3f m_eye, m_at, m_up;
    lqc::Vector3f m_z_e, m_x_e, m_y_e;
  private:
    float f, a, b;
    float aspect;

    lqc::Vector3f m_init_eye, m_init_center, m_init_iup;

    CamProjection m_projection;
    float m_r_b, m_r_h;

    ////TEMPORARIO
    float m_width, m_height;

    

    float m_fov_y, m_near_plane, m_far_plane;

    float m_mov_speed, m_rot_speed;
    ////TEMPORARIO
  };

}

#endif