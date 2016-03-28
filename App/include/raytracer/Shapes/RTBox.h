/**
* v0: link with v2 v4 and v1
* ...
* v7: link with v5 v6 and v3
*
*             y
*            /\
*            |
*            |
*            0-----------2
*           /|          /
*          / |         /|
*         /  |        / |
*        /   |       /  |
*       1-----------3   |
*       |    |      |   |
*       |    4----------6-------------->x
*       |   /       |   /
*       |  /        |  /
*       | /         | /
*       |/          |/
*       5-----------7
*      /
*     /
*    \/
*    z
**/

#ifndef RAYTRACER_BOX_H
#define RAYTRACER_BOX_H

#include <raytracer/RTRay.h>
#include <raytracer/Shapes/RTTriangle.h>
#include <raytracer/Shapes/RTObject.h>
#include <math/Vector3.h>

namespace rt
{
  class RTBox : public RTObject
  {
  public:
    RTBox ();
    RTBox (lqc::Vector3f v0, lqc::Vector3f v1, lqc::Vector3f v2, lqc::Vector3f v3,
           lqc::Vector3f v4, lqc::Vector3f v5, lqc::Vector3f v6, lqc::Vector3f v7,
           lqc::Vector3f color = lqc::Vector3f (0.7f, 0.7f, 0.0f));
    RTBox (lqc::Vector3f p_min, lqc::Vector3f p_max, lqc::Vector3f color = lqc::Vector3f (0.7f, 0.7f, 0.0f));
    ~RTBox ();

    void Create (lqc::Vector3f v0, lqc::Vector3f v1, lqc::Vector3f v2, lqc::Vector3f v3,
                 lqc::Vector3f v4, lqc::Vector3f v5, lqc::Vector3f v6, lqc::Vector3f v7,
                 lqc::Vector3f color = lqc::Vector3f (0.7f, 0.7f, 0.0f));

    virtual bool Hit (RTRay ray, float &tmin, lqc::Vector3f &normal);
    virtual bool Hit (RTRay ray, float &tmin, lqc::Vector3f &normal, lqc::Vector3f &localhitpoint);
    virtual bool Hit (RTRay ray, float &tmin, float &tmax, lqc::Vector3f &normal);
    virtual lqc::Vector3f Normal (lqc::Vector3f point);

  private:
    void MakeTriangles ();
    lqc::Vector3f m_vertex[8];
    RTTriangle m_triangles[12];
  };
}

#endif

