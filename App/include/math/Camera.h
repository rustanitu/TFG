#ifndef LQC_MATH_CAMERA_H
#define LQC_MATH_CAMERA_H

#include <math/Vector3.h>
#include <math/Quaternion.h>

namespace lqc
{
  class Camera
  {
  public:
    virtual const char* GetNameClass () { return "Camera"; }
    virtual void Init (void* data = NULL) {}
    virtual void Reset () {}
    virtual void Keyboard_cb (unsigned char key, int x, int y) {}
    virtual Vector3f CastRay (float x, float y) { return Vector3f (0.0f); }
    
    virtual void Move (float d_x, float d_y, float d_z)
    {
      m_pos = m_pos - m_mov_speed * d_z * m_z_e;
      m_pos = m_pos + m_mov_speed * d_x * m_x_e;
      m_pos = m_pos + m_mov_speed * d_y * m_y_e;
    }

    virtual void Move (Vector3f direction, float d)
    {
      m_pos = d * direction;
    }

    virtual void Rotate (float pitchdegrees, float yawdegrees, float rolldegrees) {}

  public:
    Camera (float width, float height);
    ~Camera ();

    Vector3f GetPosition ();
    void SetPosition (Vector3f);

    void SetMovementSpeed (float sp);
    float GetMovementSpeed ();

    void SetRotationSpeed (float sp);
    float GetRotationSpeed ();

  protected:
    float m_width, m_height;

    Vector3f m_eye, m_at, m_up;
    
    Vector3f m_init_eye, m_init_at, m_init_up;
    
    Vector3f m_z_e, m_x_e, m_y_e;
    Vector3f m_pos;
    bool m_outdated;

    float m_fov_y, m_near_plane, m_far_plane;

    float m_mov_speed, m_rot_speed;
  
  private:

  };


  //Precisa guardar os 3 pontos para a look at...
  class CameraQuaternion : public Camera
  {
  public:
    //class InitStruct
    //{
    //public:
    //
    //};
  public:
    virtual const char* GetNameClass () {
      return "CameraQuaternion";
    }

    virtual void Init (void* data = NULL)
    {
      if (data == NULL)
      {
        m_eye = Vector3f (0.0f, 0.0f, 0.0f);
        m_at  = Vector3f (0.0f, 0.0f, -10.0f);
        m_up  = Vector3f (0.0f, 1.0f, 0.0f);
      }
      else
      {

      }
    }
    
  public:
    CameraQuaternion (float width = 0, float height = 0)
      : Camera (width, height)
    {}

    ~CameraQuaternion ()
    {}

    void MakeRotations (float xan, float yan, float zan);

    void RotateView (Vector3f axis, float angle)
    {
      Quaternion V (m_at.x, m_at.y, m_at.z, 0);

      Quaternion R (
        axis.x * sin (angle / 2.0f),
        axis.y * sin (angle / 2.0f),
        axis.z * sin (angle / 2.0f),
        cos (angle / 2.0f)
        );

      Quaternion W = Quaternion::Multiply (
        Quaternion::Multiply (R, V), 
        Quaternion::Conjugate(R));

      m_at = Vector3f (W.x, W.y, W.z);

      //gluLookAt (Position.x, Position.y, Position.z,
      //  View.x, View.y, View.z, Up.x, Up.y, Up.z).
    }

    //http://3dgep.com/understanding-the-view-matrix/
    //http://www.songho.ca/opengl/gl_transform.html
    //https://www.opengl.org/archives/resources/faq/technical/viewing.htm
    // Translada e rotacionaa
    Matrix4f GenerateLookAtMatrix ();
    Matrix4f GenerateProjectionMatrix (float fovy, float aspect_ratio, float near_plane, float far_plane);

    void Rotate (Vector3f vector, Vector3f axis, float angle);
    Vector3f RotateAxis (Vector3f vector, Vector3f axis, float angle);

    void RotatePitch (float angle);
    void RotateYaw (float angle);
    void RotateRoll (float angle);

    void MoveForward ();
    void MoveBackward ();

    void MoveRight ();
    void MoveLeft ();

    void MoveUp ();
    void MoveDown ();

    void movex (float xmmod);
    void movey (float ymmod);
    void movez (float zmmod);
    void rotatex (float xrmod);
    void rotatey (float yrmod);
    void Update (float seconds, float* v);

    Quaternion rotation;

  protected:
  private:
    float m_fovx;
    float m_aspectRatio;
    float m_znear;
    float m_zfar;
  };

  class CameraSimpleQuaternion : public Camera
  {
  public:
    virtual const char* GetNameClass () { return "CameraSimpleQuaternion"; }
    virtual void Init (void* data = NULL) {}
  public:
    CameraSimpleQuaternion (float width = 0, float height = 0) : Camera (width, height) {}
    ~CameraSimpleQuaternion () {}
  protected:
  private:
  };

  class Camera_glm : public Camera
  {
  public:
    virtual const char* GetNameClass () {
      return "Camera_glm";
    }

    virtual void Init (void* data = NULL) {}
    virtual void Reset () {}
    virtual void Keyboard_cb (unsigned char key, int x, int y) {}
    virtual Vector3f CastRay (float x, float y) {
      return Vector3f (0.0f);
    }
    virtual void Move (float d_x, float d_y, float d_z);
    virtual void Move (Vector3f direction, float d);
    virtual void Rotate (float pitchdegrees, float yawdegrees, float rolldegrees);
    void RotateAr (float pitchdegrees, float yawdegrees, float rolldegrees, glm::mat4* model);
  public:
    Camera_glm (float width = 0, float height = 0);
    ~Camera_glm ();

    glm::mat4 GenerateLookAtMatrix ();
    glm::mat4 GenerateProjectionMatrix (float fovy, float aspect_ratio, float near_plane, float far_plane);

    void UpdateLookAtMatrix (glm::vec3 eye, glm::vec3 target, glm::vec3 up);
    glm::mat4 GetViewMatrix ();

  protected:
    glm::mat4 m_view_matrix;
  private:
  };

  void RotateAroundArbitraryAxis (glm::vec3 axis, float angle, glm::mat4* model, float speed = 0.25f);

}

#endif