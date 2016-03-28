#include <math/Camera.h>
#include <math/MGeometry.h>
#include <math/MUtils.h>

#include <math/Vector3.h>

#include <lqc/lqcdefines.h>

namespace lqc
{
  Camera::Camera (float width, float height)
    : m_width (width), m_height (height) 
  {
    m_mov_speed = 1.0;
    m_rot_speed = 1.0;
    m_z_e = Vector3f (0.0f, 0.0f, 1.0f);
    m_y_e = Vector3f (0.0f, 1.0f, 0.0f);
    m_x_e = Vector3f (1.0f, 0.0f, 0.0f);
  }
  Camera::~Camera () {}

  Vector3f Camera::GetPosition () { return m_pos; }
  void Camera::SetPosition (Vector3f pos) { m_pos = pos; }

  void Camera::SetMovementSpeed (float sp) { m_mov_speed = sp; }
  float Camera::GetMovementSpeed () { return m_mov_speed; }

  void Camera::SetRotationSpeed (float sp) { m_rot_speed = sp; }
  float Camera::GetRotationSpeed () { return m_rot_speed; }

  Matrix4f CameraQuaternion::GenerateLookAtMatrix ()
  {
    float Px = - lqc::DotProduct (m_x_e, m_pos);
    float Py = - lqc::DotProduct (m_y_e, m_pos);
    float Pz = - lqc::DotProduct (m_z_e, m_pos);

    Matrix4f m;
    m.m[0]  = m_x_e.x; m.m[1]  = m_y_e.x; m.m[2]  = m_z_e.x; m.m[3]  = 0;
    m.m[4]  = m_x_e.y; m.m[5]  = m_y_e.y; m.m[6]  = m_z_e.y; m.m[7]  = 0;
    m.m[8]  = m_x_e.z; m.m[9]  = m_y_e.z; m.m[10] = m_z_e.z; m.m[11] = 0;
    m.m[12] = Px;      m.m[13] = Py;      m.m[14] = Pz;      m.m[15] = 1;
    
    return m;
  }

  Matrix4f CameraQuaternion::GenerateProjectionMatrix (float fovy, float aspect_ratio, float near_plane, float far_plane)
  {
    Matrix4f out = { { 0 } };

    const float
      yScale = (float)lqc::Cotangent (lqc::DegreesToRadians ((double)fovy / 2.0)),
      xScale = yScale / aspect_ratio,
      frustum_length = far_plane - near_plane;

    out.m[0]  = xScale;
    out.m[1]  = 0.0f;
    out.m[2]  = 0.0f;
    out.m[3]  = 0.0f;
              
    out.m[4]  = 0.0f;
    out.m[5]  = yScale;
    out.m[6]  = 0.0f;
    out.m[7]  = 0.0f;
              
    out.m[8]  = 0.0f;
    out.m[9]  = 0.0f;
    out.m[10] = (far_plane + near_plane) / (near_plane - far_plane);
    out.m[11] = -1.0f;

    out.m[12] = 0.0f;
    out.m[13] = 0.0f;
    out.m[14] = (2.0f * near_plane * far_plane) / (near_plane - far_plane);
    out.m[15] = 0.0f;

    return out;

    /*
    float e = 1.0f / tanf (Math::degreesToRadians (fovx) / 2.0f);
    float aspectInv = 1.0f / aspect;
    float fovy = 2.0f * atanf (aspectInv / e);
    float xScale = 1.0f / tanf (0.5f * fovy);
    float yScale = xScale / aspectInv;

    m_fovx = fovx;*/
    m_aspectRatio = aspect_ratio;
    m_znear = near_plane;
    m_zfar = far_plane;
  }

  void CameraQuaternion::MakeRotations (float xan, float yan, float zan)
  {
    //m_y_e = lqc::Vector3f::Normalize (RotateAxis (m_y_e, Vector3f (1, 0, 0), m_rot_speed * xan));
    //m_x_e = lqc::Vector3f::Normalize (RotateAxis (m_x_e, Vector3f (0, 1, 0), m_rot_speed * yan));
    //m_z_e = lqc::Vector3f::Normalize (lqc::CrossProduct (m_x_e, m_y_e));
  }

  void CameraQuaternion::Rotate (Vector3f vector, Vector3f axis, float angle)
  {
    Quaternion V (vector.x, vector.y, vector.z, 0);

    Quaternion Q (
      V.x * sin (angle / 2.0f),
      V.y * sin (angle / 2.0f),
      V.z * sin (angle / 2.0f),
      cos (angle / 2.0f)
      );

    Quaternion W = ((Q * V) * Quaternion::Conjugate (Q));

    vector = Vector3f (W.x, W.y, W.z);

    //gluLookAt (Position.x, Position.y, Position.z,
    //  View.x, View.y, View.z, Up.x, Up.y, Up.z).
  }

  Vector3f CameraQuaternion::RotateAxis (Vector3f vector, Vector3f axis, float angle)
  {
    Quaternion V (vector.x, vector.y, vector.z, 0);
    float halfAngle = angle * ((float)PI / 360.0f);

    Quaternion Q (
      axis.x * sinf (halfAngle),
      axis.y * sinf (halfAngle),
      axis.z * sinf (halfAngle),
      cosf (halfAngle)
      );

    Quaternion W = ((Q * V) * Quaternion::Conjugate (Q));
    return Vector3f (W.x, W.y, W.z);
  }

  void CameraQuaternion::RotatePitch (float angle)
  {
    printf ("RotatePitch");
    rotatex (angle * m_rot_speed);
    //m_y_e = RotateAxis (m_y_e, m_x_e, angle);
    //m_z_e = RotateAxis (m_z_e, m_x_e, angle);
  }

  void CameraQuaternion::RotateYaw (float angle)
  {
    printf ("RotateYaw");
    rotatey (angle * m_rot_speed);
    //m_z_e = RotateAxis (m_z_e, m_y_e , angle);
    //m_x_e = RotateAxis (m_x_e, m_y_e, angle);
  }

  void CameraQuaternion::RotateRoll (float angle)
  {
  }

  void CameraQuaternion::MoveForward ()
  {
    m_pos = m_pos - m_mov_speed * m_z_e;
  }

  void CameraQuaternion::MoveBackward ()
  {
    m_pos = m_pos + m_mov_speed * m_z_e;
  }

  void CameraQuaternion::MoveRight ()
  {
    m_pos = m_pos + m_mov_speed * m_x_e;
  }

  void CameraQuaternion::MoveLeft ()
  {
    m_pos = m_pos - m_mov_speed * m_x_e;
  }

  void CameraQuaternion::MoveUp ()
  {
    m_pos = m_pos + m_mov_speed * m_y_e;
  }

  void CameraQuaternion::MoveDown ()
  {
    m_pos = m_pos - m_mov_speed * m_y_e;
  }

  void CameraQuaternion::movex (float xmmod)
  {
    m_pos += rotation * Vector3f (xmmod, 0.0f, 0.0f);
  }

  void CameraQuaternion::movey (float ymmod)
  {
    m_pos.y -= ymmod;
  }

  void CameraQuaternion::movez (float zmmod)
  {
    m_pos += rotation * Vector3f (0.0f, 0.0f, -zmmod);
  }

  void CameraQuaternion::rotatex (float xrmod)
  {
    Quaternion nrot (Vector3f (1.0f, 0.0f, 0.0f), xrmod * PIOVER180);
    rotation = rotation * nrot;
  }

  void CameraQuaternion::rotatey (float yrmod)
  {
    Quaternion nrot (Vector3f (0.0f, 1.0f, 0.0f), yrmod * PIOVER180);
    rotation = nrot * rotation;
  }

  void CameraQuaternion::Update (float seconds, float* v)
  {
    if (v[0] != 0.0f) rotatex (v[0] * seconds * m_rot_speed);
    if (v[1] != 0.0f) rotatey (v[1] * seconds * m_rot_speed);

    if (v[2] != 0.0f) movex (v[2] * seconds * m_mov_speed);
    if (v[3] != 0.0f) movey (v[3] * seconds * m_mov_speed);
    if (v[4] != 0.0f) movez (v[4] * seconds * m_mov_speed);
  }

}