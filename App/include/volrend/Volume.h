#ifndef VOLREND_VOLUME_H
#define VOLREND_VOLUME_H

#include <iostream>
#include <stack>
#include <math/Vector3.h>
#include <math/Vector4.h>

#include <glutils/GLTexture3D.h>
#include <volrend/TransferFunction1D.h>

class ATFGenerator;

namespace vr
{
  enum VolumeType
  {
    STRUCTURED,
    UNSTRUCTURED,
  };

  //template<class T>
  class Volume
  {
  public:
    Volume();
    Volume(unsigned int width, unsigned int height, unsigned int depth);
    Volume(unsigned int width, unsigned int height, unsigned int depth, float* scalars);
    ~Volume();

    int GetWidth();
    int GetHeight();
    int GetDepth();

    lqc::Vector3f GetAnchorMin();
    lqc::Vector3f GetAnchorMax();
    void SetAnchors(lqc::Vector3f pmin, lqc::Vector3f pmax);

    int SampleVolume(const unsigned int& x, const unsigned int& y, const unsigned int& z);
    int SampleVolume(float x, float y, float z);
    int SampleVolume(const unsigned int& id)
    {
      return m_scalar_values[id];
    }

    float InterpolatedValue(float x, float y, float z);
    float InterpolatedValue(lqc::Vector3f pos);

    float TrilinearScalarFunction(lqc::Vector3f pos, lqc::Vector3f rayeye, lqc::Vector3f raydirection);

    bool IsOutOfBoundary(int x, int y, int z);

    std::string GetName()
    {
      return m_name;
    }

    void SetName(std::string name)
    {
      m_name = name;
    }

    bool Validate()
    {
      return m_scalar_values != NULL;
    }

    int GetId(const unsigned int& x, const unsigned int& y, const unsigned int& z) const
    {
      return x + (y * m_width) + (z * m_width * m_height);
    }

    void SeparateBoundaries(vr::TransferFunction1D* atfg);

    int* GetVoxelSet()
    {
      return m_set_values;
    }

    int* GetSetQtd()
    {
      return m_set_qtd;
    }

    int GetSetQtdSize()
    {
      return m_nsets;
    }

    float* GetValues()
    {
      return m_scalar_values;
    }

  private:
    struct Voxel
    {
      int x, y, z;
    };

    int GetNeighborhod(int x, int y, int z, Voxel* neigh);

  private:
    std::string m_name;

    lqc::Vector3f m_pmin;
    lqc::Vector3f m_pmax;

    unsigned int m_width, m_height, m_depth;
    float* m_scalar_values;
    int m_nsets;
    bool* m_visited;
    int* m_set_values;
    int* m_set_qtd;
    std::stack<Voxel> m_stack;
  };
}

#endif