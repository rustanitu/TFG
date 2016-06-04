#ifndef ATFGenerator_H
#define ATFGenerator_H

#include "IATFGenerator.h"
#include <volrend/Volume.h>

class ATFGenerator
{
public:
  /* Management Functions*/
  ATFGenerator();
  ~ATFGenerator();

private:
  void Init();
  void CleanUp();

public:
  /* Getters and Setters */
  float GetGradient(int x, int y, int z);
  float GetLaplacian(int x, int y, int z);
  void SetVolume(vr::Volume* vol);

private:
  /* Transfer Function Generation Helpers */
  float CalculateGradient(int x, int y, int z);
  float CalculateLaplacian(int x, int y, int z);

  void CalculateVolumeDerivatives();

  float* GetBoundaryDistancies();
  bool GenerateHistogram();

  void GenerateHistogramSlices();
  void GenerateHistogramSlice(int v);

  void GenerateValueGradientSummedHistogram();
  void GenerateValueLaplaceSummedHistogram();

  float Sigma(unsigned char v);

public:
  virtual ITransferFunction* GetTransferFunction();

private:
  unsigned int m_width, m_height, m_depth;  // Volume dimensions.
  vr::Volume* m_volume;                     // Volume attached.

  long m_max_gradient[256];                      // Extrema gradient value.
  long m_max_laplacian[256], m_min_laplacian[256];    // Extrema laplacian value.
  long m_max_global_gradient;                      // Extrema global gradient value.
  long m_max_global_laplacian, m_min_global_laplacian;    // Extrema global laplacian value.

  float* m_scalar_gradient;                 // Voxels' gradients.
  float* m_scalar_laplacian;                // Voxels' laplacians.

  float m_average_gradient[256];            // Average gradient for each value.

  float** m_average_laplacian;               // Average laplacian for each pair value x gradient.

  unsigned char*** m_scalar_histogram;        // Histogram of occurences for each triple value x gradient x laplacian.
};

#endif