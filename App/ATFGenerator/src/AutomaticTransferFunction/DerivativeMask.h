#ifndef ATFG_DERIVATIVE_MASK
#define ATFG_DERIVATIVE_MASK

static class DerivativeMask
{
public:
  DerivativeMask(int n);

  void GetGradient(int x, int y, int z, float* gx, float* gy, float* gz);

  void GetLaplacian(int x, int y, int z, float* lx, float* ly, float* lz);

private:
  unsigned int GetId(int x, int y, int z);
  void GenerateGradientMask();
  void GenerateLaplacianMask();
private:
  int m_n;
  float *m_gaussian_kernel;
  float *m_gradient_kernel;
  float *m_laplacian_kernel;
  float *m_gradient_mask;
  float *m_laplacian_mask;
};

#endif