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

    
  private:
    float CalculateGradient (int x, int y, int z);
    float CalculateLaplacian (int x, int y, int z);
    
    float GetValue (int x, int y, int z);
    float GetGradient (int x, int y, int z);
    float GetLaplacian (int x, int y, int z);

    void FillGradientField ();
    void FillLaplacianField ();

    float* GetBoundaryDistancies ();

    bool GenerateHistogram ();
    void GenerateTransferFunction ();

  private: 
    std::string m_name;

    lqc::Vector3f m_pmin;
    lqc::Vector3f m_pmax;

    unsigned int m_width, m_height, m_depth;
    long m_max_gradient, m_max_laplacian, m_min_laplacian;
    
    float* m_scalar_values;               // Voxels' values.
    float* m_scalar_gradient;             // Voxels' gradients.
    float* m_scalar_laplacian;            // Voxels' laplacians.

    float m_average_gradient[256];        // Average gradient for each value.
    unsigned int m_value_qtd[256];
    
    float* m_average_laplacian;           // Average laplacian for each pair value x gradient.
    unsigned int* m_gradient_qtd;

    unsigned char* m_scalar_histogram;    // Histogram of occurences for each triple value x gradient x laplacian.
  };

}

#endif