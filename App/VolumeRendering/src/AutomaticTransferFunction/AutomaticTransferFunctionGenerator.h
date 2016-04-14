#ifndef ATFG_H
#define ATFG_H

/* Includes */
#include <volrend/Volume.h>

class ATFG
{
public:
  /* Management Functions*/
  ATFG();
  ~ATFG();
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

  void FillGradientField();
  void FillLaplacianField();

  float* GetBoundaryDistancies();
  bool GenerateHistogram();
public:
  void GenerateTransferFunction();

private:
  unsigned int m_width, m_height, m_depth;  // Volume dimensions.
  vr::Volume* m_volume;                     // Volume attached.

  long m_max_gradient;                      // Extrema gradient value.
  long m_max_laplacian, m_min_laplacian;    // Extrema laplacian value.

  float* m_scalar_gradient;                 // Voxels' gradients.
  float* m_scalar_laplacian;                // Voxels' laplacians.

  float m_average_gradient[256];            // Average gradient for each value.
  unsigned int m_value_qtd[256];

  float* m_average_laplacian;               // Average laplacian for each pair value x gradient.
  unsigned int* m_gradient_qtd;

  unsigned char* m_scalar_histogram;        // Histogram of occurences for each triple value x gradient x laplacian.
};

#endif