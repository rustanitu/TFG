#include "DerivativeMask.h"

#include <stdlib.h>

static const float GAUSSIANS[5][9] = {
  {1},
  {1, 2, 1},
  {1, 4, 6, 4, 1},
  {1, 6, 15, 20, 15, 6, 1},
  {1, 8, 28, 56, 70, 56, 28, 8, 1}
};

DerivativeMask::DerivativeMask(int n)
: m_n(n)
, m_gradient_mask(NULL)
, m_laplacian_mask(NULL)
{
  if (n % 2 == 0)
    m_n = n + 1;

  int i;
  int v;
  int vsum;
  int half = m_n / 2;

  // Initializes gaussian kernel acording to the specified size
  m_gaussian_kernel = new float[m_n];
  vsum = 0;
  for (i = 0; i < m_n; i++)
  {
    m_gaussian_kernel[i] = GAUSSIANS[m_n / 2][i];
    vsum += m_gaussian_kernel[i];
  }

  for (i = 0; i < m_n; i++) {
    m_gaussian_kernel[i] = m_gaussian_kernel[i] / vsum;
  }

  // Initializes gradient kernel acording to the specified size
  m_gradient_kernel = new float[m_n];
  v = 1;
  vsum = 0;
  for (i = 0; i < half; i++) {
    m_gradient_kernel[i] = -v;
    m_gradient_kernel[m_n - i - 1] = v;
    vsum += 2 * v;
    v *= 2;
  }
  m_gradient_kernel[i] = 0.0f;

  for (i = 0; i < m_n; i++) {
    m_gradient_kernel[i] /= vsum;
  }

  // Initializes laplacian kernel acording to the specified size
  m_laplacian_kernel = new float[m_n];
  v = 1;
  vsum = 0;
  for (i = 0; i < half; i++) {
    m_laplacian_kernel[i] = v;
    m_laplacian_kernel[m_n - i - 1] = v;
    vsum += 2 * v;
    v *= 2;
  }
  m_laplacian_kernel[i] = -vsum;
  vsum *= 2;

  for (i = 0; i < m_n; i++) {
    m_laplacian_kernel[i] /= vsum;
  }

  GenerateGradientMask();
  GenerateLaplacianMask();
}

unsigned int DerivativeMask::GetId(int x, int y, int z)
{
  if (x < 0)
    x = -x;
  else if (x >= m_n)
    x = 2 * m_n - 1 - x;

  if (y < 0)
    y = -y;
  else if (y >= m_n)
    y = 2 * m_n - 1 - y;

  if (z < 0)
    z = -z;
  else if (z >= m_n)
    z = 2 * m_n - 1 - z;

  return x + (y * m_n) + (z * m_n * m_n);
}

void DerivativeMask::GenerateGradientMask()
{
  delete m_gradient_mask;
  m_gradient_mask = new float[m_n * m_n * m_n];

  for (int x = 0; x < m_n; x++) {
    float dk_val = m_gradient_kernel[x];
    for (int y = 0; y < m_n; y++) {
      float dgy_val = m_gaussian_kernel[y];
      for (int z = 0; z < m_n; z++) {
        float dgz_val = m_gaussian_kernel[z];
        m_gradient_mask[GetId(x, y, z)] = dk_val * dgy_val * dgz_val;
      }
    }
  }
}

void DerivativeMask::GenerateLaplacianMask()
{
  delete m_laplacian_mask;
  m_laplacian_mask = new float[m_n * m_n * m_n];

  for (int x = 0; x < m_n; x++) {
    float dk_val = m_laplacian_kernel[x];
    for (int y = 0; y < m_n; y++) {
      float dgy_val = m_gaussian_kernel[y];
      for (int z = 0; z < m_n; z++) {
        float dgz_val = m_gaussian_kernel[z];
        m_laplacian_mask[GetId(x, y, z)] = dk_val * dgy_val * dgz_val;
      }
    }
  }
}

void DerivativeMask::GetGradient(int x, int y, int z, float* gx, float* gy, float* gz)
{
  if (x < 0 || y < 0 || z < 0 || x >= m_n || y >= m_n || z >= m_n) {
    gx = gy = gz = NULL;
    return;
  }

  *gx = m_gradient_mask[GetId(x, y, z)];
  *gy = m_gradient_mask[GetId(y, x, z)];
  *gz = m_gradient_mask[GetId(z, y, x)];
}

void DerivativeMask::GetLaplacian(int x, int y, int z, float* lx, float* ly, float* lz)
{
  if (x < 0 || y < 0 || z < 0 || x >= m_n || y >= m_n || z >= m_n) {
    lx = ly = lz = NULL;
    return;
  }

  *lx = m_laplacian_mask[GetId(x, y, z)];
  *ly = m_laplacian_mask[GetId(y, x, z)];
  *lz = m_laplacian_mask[GetId(z, y, x)];
}