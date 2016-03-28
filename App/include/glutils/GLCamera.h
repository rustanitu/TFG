#ifndef LQC_GL_CAMERA_H
#define LQC_GL_CAMERA_H

#include <math/Camera.h>
#include <math/Matrix4.h>
#include <math/Vector4.h>
#include <math/Vector3.h>

namespace gl
{
  class GLCamera : public lqc::Camera
  {
  public:
    virtual const char* GetNameClass () { return "GLCamera"; }

    virtual void Init (void* data = NULL);

  public:
    enum CAM_MOVE{
      FORWARD,
      BACK   ,
      LEFT   ,
      RIGHT  ,
      UP     ,
      DOWN   ,
    };

    GLCamera ();
    GLCamera (int WindowWidth, int WindowHeight);
    GLCamera (int WindowWidth, int WindowHeight, const lqc::Vector3f& Pos, const lqc::Vector3f& Target, const lqc::Vector3f& Up);
    ~GLCamera ();

    void CalculateViewMatrix ();
    void LookAt (lqc::Vector3f Reference, lqc::Vector3f Position, bool RotateAroundReference = false);
    void SetViewMatrixPointer (float *View);

    void move (CAM_MOVE direction);
    void rotate (lqc::Vector3f axis, float angle);
    
    lqc::Matrix4f getViewMatrix ();
    //TODO
    lqc::Matrix4f LookAt ();

    lqc::Vector4f getEye ();

    void Rotate (float angle, float axis_x, float axis_y, float axis_z);
    
    lqc::Vector3f m_up;

  private:
    lqc::Matrix4f *View;
    lqc::Matrix4f ViewMatrix (const lqc::Vector3f &x, const lqc::Vector3f &y, const lqc::Vector3f &z, const lqc::Vector3f &position);
  };

}

#endif


/*

#ifndef CAMERA_H
#define	CAMERA_H

#include "math_3d.h"


class Camera
{
public:

    Camera(int WindowWidth, int WindowHeight);

    Camera(int WindowWidth, int WindowHeight, const Vector3f& Pos, const Vector3f& Target, const Vector3f& Up);

    bool OnKeyboard(int Key);

    void OnMouse(int x, int y);

    void OnRender();

    const Vector3f& GetPos() const
    {
        return m_pos;
    }

    const Vector3f& GetTarget() const
    {
        return m_target;
    }

    const Vector3f& GetUp() const
    {
        return m_up;
    }

private:

    void Init();
    void Update();

    Vector3f m_pos;
    Vector3f m_target;
    Vector3f m_up;

    int m_windowWidth;
    int m_windowHeight;

    float m_AngleH;
    float m_AngleV;

    bool m_OnUpperEdge;
    bool m_OnLowerEdge;
    bool m_OnLeftEdge;
    bool m_OnRightEdge;

    Vector2i m_mousePos;
};
*/

/*

class CCamera
{
protected:
	mat4x4 *View;

public:
	vec3 X, Y, Z, Reference, Position;

	CCamera();
	~CCamera();

	void CalculateViewMatrix();
	void LookAt(vec3 Reference, vec3 Position, bool RotateAroundReference = false);
	void Move(vec3 Movement);
	vec3 OnKeys(BYTE Keys, float FrameTime);
	void OnMouseMove(int dx, int dy);
	void OnMouseWheel(float zDelta);
	void SetViewMatrixPointer(float *View);
};
*/