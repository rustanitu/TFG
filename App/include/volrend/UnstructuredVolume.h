#ifndef VOLREND_UNSTRUCTUREDVOLUME_H
#define VOLREND_UNSTRUCTUREDVOLUME_H

#include <iostream>
#include <lqc/Math/Vector3.h>
#include <lqc/Math/Vector4.h>

#include <volrend/Volume.h>

namespace vr
{
  class UnstructuredVolume
  {
  public:
    class Hexa
    {
    public:
      lqc::Vector3d vert[8];
    };

  public:
    UnstructuredVolume () {}
    ~UnstructuredVolume () {}

    std::string GetName ()
    {
      return m_name;
    }

    void SetName (std::string name)
    {
      m_name = name;
    }

  private:
    std::string m_name;
    Hexa* m_hexas;
  };
}

#endif