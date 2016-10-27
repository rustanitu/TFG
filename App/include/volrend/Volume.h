#ifndef VOLREND_VOLUME_H
#define VOLREND_VOLUME_H

#include <iostream>
#include <stack>
#include <math/Vector3.h>
#include <math/Vector4.h>

#include <glutils/GLTexture3D.h>
#include <volrend/ScalarField.h>
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
	class Volume : public ScalarField
  {
  public:
    Volume();
    Volume(const UINT32& width, const UINT32& height, const UINT32& depth);
    Volume(const UINT32& width, const UINT32& height, const UINT32& depth, float* scalars);
    ~Volume();

    lqc::Vector3f GetAnchorMin();
    lqc::Vector3f GetAnchorMax();
    void SetAnchors(lqc::Vector3f pmin, lqc::Vector3f pmax);

		float GetValue(const UINT32& x, const UINT32& y, const UINT32& z);
    float GetValue(float x, float y, float z);
		float GetValue(const UINT32& id)
    {
      return m_scalar_values[id];
    }

		float CalculateGradient(const UINT32& x, const UINT32& y, const UINT32& z);
		float CalculateLaplacian(const UINT32& x, const UINT32& y, const UINT32& z);

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

		float* m_scalar_values;
    int m_nsets;
    bool* m_visited;
    int* m_set_values;
    int* m_set_qtd;
    std::stack<Voxel> m_stack;
  };
}

#endif