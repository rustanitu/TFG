#include <raytracer/RTRay.h>
#include <raytracer/Shapes/RTBox.h>

namespace rt
{

  RTBox::RTBox ()
    : RTObject (RTObject::OBJECT_TYPE::BOX)
  {}

  RTBox::RTBox (lqc::Vector3f v0, lqc::Vector3f v1, lqc::Vector3f v2, lqc::Vector3f v3,
                lqc::Vector3f v4, lqc::Vector3f v5, lqc::Vector3f v6, lqc::Vector3f v7, lqc::Vector3f _color)
    : RTObject (RTObject::OBJECT_TYPE::BOX)
  {
    color = _color;
    m_vertex[0] = v0; m_vertex[1] = v1;
    m_vertex[2] = v2; m_vertex[3] = v3;
    m_vertex[4] = v4; m_vertex[5] = v5;
    m_vertex[6] = v6; m_vertex[7] = v7;
    MakeTriangles ();
  }

  RTBox::RTBox (lqc::Vector3f p_min, lqc::Vector3f p_max, lqc::Vector3f _color)
    : RTObject (RTObject::OBJECT_TYPE::BOX)
  {
    color = _color;
    m_vertex[0] = lqc::Vector3f (p_min.x, p_max.y, p_min.z);
    m_vertex[1] = lqc::Vector3f (p_min.x, p_max.y, p_max.z);
    m_vertex[2] = lqc::Vector3f (p_max.x, p_max.y, p_min.z);
    m_vertex[3] = lqc::Vector3f (p_max.x, p_max.y, p_max.z);
    m_vertex[4] = lqc::Vector3f (p_min.x, p_min.y, p_min.z);
    m_vertex[5] = lqc::Vector3f (p_min.x, p_min.y, p_max.z);
    m_vertex[6] = lqc::Vector3f (p_max.x, p_min.y, p_min.z);
    m_vertex[7] = lqc::Vector3f (p_max.x, p_min.y, p_max.z);
    MakeTriangles ();
  }

  RTBox::~RTBox ()
  {}

  void RTBox::Create (lqc::Vector3f v0, lqc::Vector3f v1, lqc::Vector3f v2, lqc::Vector3f v3,
                      lqc::Vector3f v4, lqc::Vector3f v5, lqc::Vector3f v6, lqc::Vector3f v7, lqc::Vector3f _color)
  {
    color = _color;
    m_vertex[0] = v0; m_vertex[1] = v1;
    m_vertex[2] = v2; m_vertex[3] = v3;
    m_vertex[4] = v4; m_vertex[5] = v5;
    m_vertex[6] = v6; m_vertex[7] = v7;
    MakeTriangles ();
  }

  bool RTBox::Hit (RTRay ray, float &tmin, lqc::Vector3f &normalv)
  {
    bool ret = false;
    bool firsthit = true;
    lqc::Vector3f retnormal;
    float rettmin;
    for (int i = 0; i < 12; i++)
    {
      if (m_triangles[i].Hit (ray, rettmin, retnormal))
      {
        if (firsthit)
        {
          tmin = rettmin;
          normalv = retnormal;
          firsthit = false;
          ret = true;
        }
        else
        {
          if (rettmin < tmin)
          {
            tmin = rettmin;
            normalv = retnormal;
          }
        }
      }
    }
    return ret;
  }

  bool RTBox::Hit (RTRay ray, float &tmin, lqc::Vector3f &normal, lqc::Vector3f &localhitpoint)
  {
    bool ret = false;
    bool firsthit = true;
    lqc::Vector3f retnormal;
    float rettmin;
    lqc::Vector3f retlocalhitpoint;
    for (int i = 0; i < 12; i++)
    {
      if (m_triangles[i].Hit (ray, rettmin, retnormal, retlocalhitpoint))
      {
        if (firsthit)
        {
          tmin = rettmin;
          normal = retnormal;
          localhitpoint = retlocalhitpoint;
          firsthit = false;
          ret = true;
        }
        else
        {
          if (rettmin < tmin)
          {
            tmin = rettmin;
            normal = retnormal;
            localhitpoint = retlocalhitpoint;
          }
        }
      }
    }
    return ret;
  }

  bool RTBox::Hit (RTRay ray, float &tmin, float &tmax, lqc::Vector3f &normal)
  {
    bool ret = false;
    bool firstset = false;
    
    float tx;
    for (int i = 0; i < 12; i++)
    {
      if (m_triangles[i].Hit (ray, tx, normal))
      {
        if (!firstset)
        {
          tmin = tx;
          firstset = true;
        }
        if (tmin != tx)
        {
          if (tmin > tx)
          {
            tmax = tmin;
            tmin = tx;
          }
          else
          {
            tmax = tx;
          }
        }
      }
    }
    return ret;
  }

  lqc::Vector3f RTBox::Normal (lqc::Vector3f point)
  {
    return lqc::Vector3f (0, 0, 0);
  }

  void RTBox::MakeTriangles ()
  {
    m_triangles[0] = RTTriangle (m_vertex[0], m_vertex[2], m_vertex[4]);
    m_triangles[1] = RTTriangle (m_vertex[4], m_vertex[2], m_vertex[6]);
    m_triangles[2] = RTTriangle (m_vertex[1], m_vertex[5], m_vertex[3]);
    m_triangles[3] = RTTriangle (m_vertex[3], m_vertex[5], m_vertex[7]);
    
    m_triangles[4] = RTTriangle (m_vertex[0], m_vertex[1], m_vertex[2]);
    m_triangles[5] = RTTriangle (m_vertex[2], m_vertex[1], m_vertex[3]);
    m_triangles[6] = RTTriangle (m_vertex[4], m_vertex[7], m_vertex[5]);
    m_triangles[7] = RTTriangle (m_vertex[4], m_vertex[6], m_vertex[7]);
    
    m_triangles[8] = RTTriangle (m_vertex[0], m_vertex[4], m_vertex[1]);
    m_triangles[9] = RTTriangle (m_vertex[1], m_vertex[4], m_vertex[5]);
    m_triangles[10] = RTTriangle (m_vertex[2], m_vertex[3], m_vertex[7]);
    m_triangles[11] = RTTriangle (m_vertex[7], m_vertex[6], m_vertex[2]);
  }
}