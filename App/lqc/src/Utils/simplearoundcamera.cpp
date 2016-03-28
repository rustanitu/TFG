#include <lqc/Utils/simplearoundcamera.h>

#include <cstdio>

SimpleAroundCamera::SimpleAroundCamera (float _radius, float _min_rad, float _max_rad)
{
  radius = _radius;
  speed = 0.001f;
  speed_radius = 0.5f;

  min_radius = _min_rad;
  max_radius = _max_rad;
}

SimpleAroundCamera::~SimpleAroundCamera ()
{}

void SimpleAroundCamera::SetInitialValues (glm::vec3 _center, glm::vec3 _up)
{
  center = _center;
  eye = glm::vec3(_center.x, _center.y, _center.z + radius);
  up = _up;
}

int SimpleAroundCamera::MouseButton (int bt, int st, int x, int y)
{
  if (st == 0 && bt == 0) {
    arcball_on = true;
    changing_radius = false;
    
    last_my = cur_my = y;
    last_mx = cur_mx = x;
  }
  else if (st == 0 && bt == 2) {
    changing_radius = true;
    arcball_on = false;

    last_my = cur_my = y;
    last_mx = cur_mx = x;
  }
  //////////////////////////////////////////
  //////////////////////////////////////////
  else if (st == 1 && bt == 0)
    arcball_on = false;
  else if (st == 1 && bt == 2)
    changing_radius = false;

  return 0;
}

int SimpleAroundCamera::MouseMotion(int x, int y)
{
  if (arcball_on) {
    float xrot = -(y - last_my) * speed;
    float yrot = -(x - last_mx) * speed;

    glm::quat p = glm::quat (0, eye.x, eye.y, eye.z);

    glm::quat qy = glm::quat (cos (yrot), sin (yrot)*up);

    glm::vec3 loc_up = up;

    float max = 0.99f;
    float dt = glm::dot (glm::normalize (glm::vec3 (center - eye)), loc_up);
    if ((dt > max && xrot > 0.0f) || (dt < -max && xrot < 0.0f))
      xrot = 0.0f;

    glm::vec3 vr = glm::normalize (glm::cross (glm::normalize (glm::vec3 (center - eye)), loc_up));
    glm::quat qx = glm::quat (cos (xrot), sin (xrot)*vr);

    glm::quat rq =
      glm::cross (glm::cross (glm::cross (glm::cross (qx, qy), p),
      glm::inverse (qy)), glm::inverse(qx));

    eye = glm::vec3(rq.x, rq.y, rq.z);


    last_mx = cur_mx;
    last_my = cur_my;
    cur_mx = x;
    cur_my = y;
  }
  //////////////////////////////////////////
  //////////////////////////////////////////
  if (changing_radius) {
    float ydiff = (y - last_my) * speed_radius;

    radius += ydiff;
    if (radius < min_radius)
      radius = min_radius;
    if (radius > max_radius)
      radius = max_radius;


    glm::vec3 c_e = glm::normalize(glm::vec3 (eye - center));

    eye = c_e * radius;

    last_my = cur_my;
    cur_my = y;
  }
  
  return 0;
}

glm::mat4 SimpleAroundCamera::LookAt()
{ 
  return glm::lookAt(eye, center, up);
}