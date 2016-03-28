#include <math/Camera.h>

#include <math/MGeometry.h>
#include <math/MUtils.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace lqc
{
  Camera_glm::Camera_glm (float width, float height)
    : Camera (width, height)
  {}

  Camera_glm::~Camera_glm ()
  {}
  
  void Camera_glm::Move (float dx, float dy, float dz)
  {
    m_pos = m_pos - m_mov_speed * dz * m_z_e;
    m_pos = m_pos + m_mov_speed * dx * m_x_e;
    m_pos = m_pos + m_mov_speed * dy * m_y_e;
  }

  void Camera_glm::Move (Vector3f direction, float d)
  {
    m_pos = d * direction;
  }

  void Camera_glm::Rotate (float pitchdegrees, float yawdegrees, float rolldegrees)
  {

  }

  glm::mat4 RotacionaEixoArb (glm::vec3 axis, float angle)
  {
    float x = axis.x;
    float y = axis.y;
    float z = axis.z;
    float c = cosf (DEGTORAD (angle));
    float s = sinf (DEGTORAD (angle));

    //axis.print();
    glm::mat4 m;
    m[0][0] = (x * x) * (1.0f - c) + c;
    m[0][1] = (x * y) * (1.0f - c) + (z * s);
    m[0][2] = (x * z) * (1.0f - c) - (y * s);
    m[0][3] = 0.0f;

    m[1][0] = (y * x) * (1.0f - c) - (z * s);
    m[1][1] = (y * y) * (1.0f - c) + c;
    m[1][2] = (y * z) * (1.0f - c) + (x * s);
    m[1][3] = 0.0f;

    m[2][0] = (z * x) * (1.0f - c) + (y * s);
    m[2][1] = (z * y) * (1.0f - c) - (x * s);
    m[2][2] = (z * z) * (1.0f - c) + c;
    m[2][3] = 0.0f;

    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;
    return m;
  }

  void Camera_glm::RotateAr (float yawdegrees, float pitchdegrees, float rolldegrees, glm::mat4* model)
  {
    float angle = 0.0f;
  
    if (yawdegrees != 0)
    {
      angle = yawdegrees;
      glm::mat4 m = RotacionaEixoArb (glm::vec3 (0, 1, 0), angle);
      (*model) = m * (*model);
    }
    if (rolldegrees != 0)
    {
      angle = rolldegrees;
      glm::mat4 m = RotacionaEixoArb (glm::vec3 (0, 0, 1), angle);
      (*model) = m * (*model);
    }
    if (pitchdegrees != 0)
    {
      angle = pitchdegrees;
      glm::mat4 m = RotacionaEixoArb (glm::vec3 (1, 0, 0), angle);
      (*model) = m * (*model);
    }
  }

  glm::mat4 Camera_glm::GenerateLookAtMatrix ()
  {
    return glm::lookAt (glm::vec3 (0, 0, 10), glm::vec3 (0, 0, 0), glm::vec3 (0, 1, 0));
  }

  //glm::mat4[col][row];
  glm::mat4 Camera_glm::GenerateProjectionMatrix (float fovy, float aspect_ratio, float near_plane, float far_plane)
  {
    return glm::perspective (fovy, aspect_ratio, near_plane, far_plane);
  }
  
  void Camera_glm::UpdateLookAtMatrix (glm::vec3 eye, glm::vec3 target, glm::vec3 up)
  {
    m_view_matrix = glm::lookAt (eye, target, up);
  }

  glm::mat4 Camera_glm::GetViewMatrix ()
  {
    return m_view_matrix;
  }

  void RotateAroundArbitraryAxis (glm::vec3 axis, float angle, glm::mat4* model, float speed)
  {
    (*model) = RotacionaEixoArb (axis, speed * angle) * (*model);
  }
}