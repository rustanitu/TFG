#ifndef VOLREND_VOLUME_H
#define VOLREND_VOLUME_H

#include <iostream>
#include <math/Vector3.h>
#include <math/Vector4.h>

#include <glutils/GLTexture3D.h>

namespace vr
{
  class TransferFunction;

  enum VolumeType
  {
    STRUCTURED,
    UNSTRUCTURED,
  };

  //template<class T>
  class Volume
  {
  public:
    Volume ();
    Volume (unsigned int width, unsigned int height, unsigned int depth);
    Volume (unsigned int width, unsigned int height, unsigned int depth, float* scalars);
    ~Volume ();

    int GetWidth ();
    int GetHeight ();
    int GetDepth ();

    lqc::Vector3f GetAnchorMin ();
    lqc::Vector3f GetAnchorMax ();
    void SetAnchors (lqc::Vector3f pmin, lqc::Vector3f pmax);

    int SampleGradient (int x, int y, int z);
    int SampleVolume (int x, int y, int z);
    int SampleVolume (float x, float y, float z);
    int SampleVolume (int id)
    {
      return m_scalar_values[id];
    }

    float InterpolatedValue (float x, float y, float z);
    float InterpolatedValue (lqc::Vector3f pos);

    float TrilinearScalarFunction (lqc::Vector3f pos, lqc::Vector3f rayeye, lqc::Vector3f raydirection);

    bool IsOutOfBoundary (int x, int y, int z);

    std::string GetName ()
    {
      return m_name;
    }

    void SetName (std::string name)
    {
      m_name = name;
    }

    bool Validate ()
    {
      return m_scalar_values != NULL;
    }

    void FillGradientField ();

  private:
    int GradientSample (int x, int y, int z);

  private: 
    std::string m_name;

    lqc::Vector3f m_pmin;
    lqc::Vector3f m_pmax;

    unsigned int m_width, m_height, m_depth;
    float* m_scalar_values;
    float* m_scalar_gradient;

    lqc::Vector3f GenGradientSampleNxNxN (int x, int y, int z, int n);
  };

}

#endif