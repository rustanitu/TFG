#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

class SimpleAroundCamera
{
public:
  SimpleAroundCamera (float _radius, float _min_rad = 20.0f, float _max_rad = 980.0f);
  ~SimpleAroundCamera ();

  void SetInitialValues (glm::vec3 _center, glm::vec3 _up);

  int MouseButton (int bt, int st, int x, int y);

  int MouseMotion (int x, int y);
  
  glm::mat4 LookAt ();

protected:
  glm::vec3 eye,center,up;
  float radius;
  float speed;
  float speed_radius;

  float min_radius;
  float max_radius;

private:
  int last_mx, last_my;
  int cur_mx, cur_my;
  bool arcball_on;
  bool changing_radius;
};