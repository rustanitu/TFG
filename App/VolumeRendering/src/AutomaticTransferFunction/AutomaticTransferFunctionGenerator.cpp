#include "AutomaticTransferFunctionGenerator.h"

#include <cstdlib>
#include <iostream>
#include <fstream>

#include <lqc/File/RAWLoader.h>
#include <math/MUtils.h>


/*************************/
/* Constants Definitions */
/*************************/

#define SQRT_E 1.6487212707
#define MAX_V 256

const int SOBEL_X_MASK[27] =
{
  -1, -2, -1, -2, -4, -2, -1, -2, -1,
   0,  0,  0,  0,  0,  0,  0,  0,  0,
   1,  2,  1,  2,  4,  2,  1,  2,  1
};

const int SOBEL_Y_MASK[27] =
{
  -1, -2, -1,  0,  0,  0,  1,  2,  1,
  -2, -4, -2,  0,  0,  0,  2,  4,  2,
  -1, -2, -1,  0,  0,  0,  1,  2,  1
};

const int SOBEL_Z_MASK[27] =
{
  -1,  0,  1, -2,  0,  2, -1,  0,  1,
  -2,  0,  2, -4,  0,  4, -2,  0,  2,
  -1,  0,  1, -2,  0,  2, -1,  0,  1
};

const int LAPLACE_MASK[27] =
{
   0,  0,  0,  0,  1,  0,  0,  0,  0,
   0,  1,  0,  1, -6,  1,  0,  1,  0,
   0,  0,  0,  0,  1,  0,  0,  0,  0
};


/************************/
/* Management Functions */
/************************/

ATFG::ATFG()
:m_width(0), m_height(0), m_depth(0), m_max_gradient(-LONG_MAX), m_min_laplacian(LONG_MAX), m_max_laplacian(-LONG_MAX),
 m_scalar_gradient(NULL), m_scalar_laplacian(NULL), m_scalar_histogram(NULL), m_average_laplacian(NULL), m_gradient_qtd(NULL)
{
}


ATFG::~ATFG()
{
  CleanUp();
}

void ATFG::Init()
{
  m_scalar_gradient = NULL;
  m_scalar_gradient = new float[m_width*m_height*m_depth];

  m_scalar_laplacian = NULL;
  m_scalar_laplacian = new float[m_width*m_height*m_depth];

  for (unsigned int i = 0; i < m_width*m_height*m_depth; i++)
  {
    m_scalar_gradient[i] = 0.0f;
    m_scalar_laplacian[i] = 0.0f;
  }
}

void ATFG::CleanUp()
{
  m_width = 0;
  m_height = 0;
  m_depth = 0;
  m_max_gradient = -LONG_MAX;
  m_max_laplacian = -LONG_MAX;
  m_min_laplacian = LONG_MAX;

  for (int v = 0; v < MAX_V; ++v)
  {
    m_average_gradient[v] = 0.0f;
    m_value_qtd[v] = 0.0f;
  }
  
  if (m_scalar_gradient)
  {
    delete m_scalar_gradient;
    m_scalar_gradient = NULL;
  }

  if (m_scalar_laplacian)
  {
    delete m_scalar_laplacian;
    m_scalar_laplacian = NULL;
  }

  if (m_average_laplacian)
  {
    delete m_average_laplacian;
    m_average_laplacian = NULL;
  }

  if (m_gradient_qtd)
  {
    delete m_gradient_qtd;
    m_gradient_qtd = NULL;
  }

  if (m_scalar_histogram)
  {
    delete m_scalar_histogram;
    m_scalar_histogram = NULL;
  }

  m_volume = NULL;
}


/************************/
/* Getters and Setters */
/************************/

float ATFG::GetGradient(int x, int y, int z)
{
  x = lqc::Clamp(x, 0, m_width - 1);
  y = lqc::Clamp(y, 0, m_height - 1);
  z = lqc::Clamp(z, 0, m_depth - 1);

  return m_scalar_gradient[x + (y * m_width) + (z * m_width * m_height)];
}

float ATFG::GetLaplacian(int x, int y, int z)
{
  x = lqc::Clamp(x, 0, m_width - 1);
  y = lqc::Clamp(y, 0, m_height - 1);
  z = lqc::Clamp(z, 0, m_depth - 1);

  return m_scalar_laplacian[x + (y * m_width) + (z * m_width * m_height)];
}

void ATFG::SetVolume(vr::Volume* vol)
{
  CleanUp();

  m_volume = vol;
  m_width = vol->GetWidth();
  m_height = vol->GetHeight();
  m_depth = vol->GetDepth();

  Init();
  FillGradientField();
  FillLaplacianField();
  GenerateHistogram();
}


/****************************************/
/* Transfer Function Generation Helpers */
/****************************************/

float ATFG::CalculateGradient(int x, int y, int z)
{
  //int s = 0;
  float glx = 0.5f * (m_volume->GetValue(x + 1, y, z) - m_volume->GetValue(x - 1, y, z));
  float gly = 0.5f * (m_volume->GetValue(x, y + 1, z) - m_volume->GetValue(x, y - 1, z));
  float glz = 0.5f * (m_volume->GetValue(x, y, z + 1) - m_volume->GetValue(x, y, z - 1));

  //float g = 0.0f;
  //float gx = 0.0f;
  //float gy = 0.0f;
  //float gz = 0.0f;
  //float kernel[3] = { -0.5f, 0.0f, 0.5f };
  //for (int i = x - 1; i <= x + 1; ++i)
  //{
  //  for (int j = y - 1; j <= y + 1; ++j)
  //  {
  //    for (int k = z - 1; k <= z + 1; ++k)
  //    {
  //      float v = m_volume->GetValue(i, j, k);
  //      g += v * kernel[i - x + 1] * kernel[j - y + 1] * kernel[k - z + 1];
  //      gx += v * kernel[i - x + 1];
  //      gy += v * kernel[j - y + 1];
  //      gz += v * kernel[k - z + 1];
  //      //gx += SOBEL_X_MASK[s] * v;
  //      //gy += SOBEL_Y_MASK[s] * v;
  //      //gz += SOBEL_Z_MASK[s] * v;
  //      //++s;
  //    }
  //  }
  //}

  //for (int i = x - 1; i <= x + 1; ++i)
  //{
  //  for (int j = y - 1; j <= y + 1; ++j)
  //  {
  //    for (int k = z - 1; k <= z + 1; ++k)
  //    {
  //      float v = m_volume->GetValue(i, j, k);
  //      gx += SOBEL_X_MASK[s] * v;
  //      gy += SOBEL_Y_MASK[s] * v;
  //      gz += SOBEL_Z_MASK[s] * v;
  //      ++s;
  //    }
  //  }
  //}

  //gx /= 16.0f;
  //gy /= 16.0f;
  //gz /= 16.0f;

  //g = sqrt(gx*gx + gy*gy + gz*gz);
  float g = sqrt(glx*glx + gly*gly + glz*glz);
  // Comparar g e gg
  g = fmax(0.0f, g);
  if (g > m_max_gradient)
    m_max_gradient = g + 0.5f;

  return g;
}

float ATFG::CalculateLaplacian(int x, int y, int z)
{
  //int s = 0;
  float g = 2 * m_volume->GetValue(x, y, z);
  float lx = m_volume->GetValue(x + 1, y, z) - g + m_volume->GetValue(x - 1, y, z);
  float ly = m_volume->GetValue(x, y + 1, z) - g + m_volume->GetValue(x, y - 1, z);
  float lz = m_volume->GetValue(x, y, z + 1) - g + m_volume->GetValue(x, y, z - 1);

  //float lx = 0.25f * (m_volume->GetValue(x + 1, y, z) - m_volume->GetValue(x - 1, y, z));
  //float ly = 0.25f * (m_volume->GetValue(x, y + 1, z) - m_volume->GetValue(x, y - 1, z));
  //float lz = 0.25f * (m_volume->GetValue(x, y, z + 1) - m_volume->GetValue(x, y, z - 1));

  //float lx = 0.0f;
  //float ly = 0.0f;
  //float lz = 0.0f;

  //float l = 0.0f;
  //float kernel[3] = { -1.0f, 2.0f, 1.0f };
  //for (int i = x - 1; i <= x + 1; ++i) {
  //  for (int j = y - 1; j <= y + 1; ++j) {
  //    for (int k = z - 1; k <= z + 1; ++k) {
  //      float v = m_volume->GetValue(i, j, k);
  //      l += v * kernel[i - x + 1] * kernel[j - y + 1] * kernel[k - z + 1];
  //      lx += v * kernel[i - x + 1];
  //      ly += v * kernel[j - y + 1];
  //      lz += v * kernel[k - z + 1];
  //      //gx += SOBEL_X_MASK[s] * v;
  //      //gy += SOBEL_Y_MASK[s] * v;
  //      //gz += SOBEL_Z_MASK[s] * v;
  //      //++s;
  //    }
  //  }
  //}

  //for (int i = x - 1; i <= x + 1; ++i)
  //{
  //  for (int j = y - 1; j <= y + 1; ++j)
  //  {
  //    for (int k = z - 1; k <= z + 1; ++k)
  //    {
  //      float v = m_scalar_gradient[i + (j * m_width) + (k * m_width * m_height)];// m_volume->GetValue(i, j, k);
  //      lx += SOBEL_X_MASK[s] * v;
  //      ly += SOBEL_Y_MASK[s] * v;
  //      lz += SOBEL_Z_MASK[s] * v;
  //      //++s;
  //      //lx += LAPLACE_MASK[s] * m_volume->GetValue(i, j, k);
  //      //++s;
  //    }
  //  }
  //}

  //lx /= 4.0f;
  //ly /= 4.0f;
  //lz /= 4.0f;

  //float l = sqrt(lx*lx + ly*ly + lz*lz);
  float l = lx + ly + lz;
  if (l > m_max_laplacian)
    m_max_laplacian = l + 0.5f;
  if (l < m_min_laplacian)
    m_min_laplacian = l - 0.5f;

  return l;
}

void ATFG::FillGradientField()
{
  for (int x = 1; x < m_width - 1; x++)
  {
    for (int y = 1; y < m_height - 1; y++)
    {
      for (int z = 1; z < m_depth - 1; z++)
      {
        m_scalar_gradient[x + (y * m_width) + (z * m_width * m_height)] = CalculateGradient(x, y, z);
        m_scalar_laplacian[x + (y * m_width) + (z * m_width * m_height)] = CalculateLaplacian(x, y, z);
      }
    }
  }
}

void ATFG::FillLaplacianField()
{
  return;
  for (int x = 1; x < m_width - 1; x++)
  {
    for (int y = 1; y < m_height - 1; y++)
    {
      for (int z = 1; z < m_depth - 1; z++)
      {
        m_scalar_laplacian[x + (y * m_width) + (z * m_width * m_height)] = CalculateLaplacian(x, y, z);
      }
    }
  }
}

float* ATFG::GetBoundaryDistancies()
{
  float sigma = m_max_gradient / ((float)m_max_laplacian * SQRT_E);

  float* x = new float[MAX_V];

  for (size_t v = 0; v < MAX_V; ++v)
  {
    float g_tresh = m_average_gradient[v] * 0.1f;
    float g = m_average_gradient[v];
    float l = m_average_laplacian[v][(unsigned long)g];
    if (g <= 0.0f)
    {
      x[v] = 0.0f;
      continue;
    }

    x[v] = -sigma * sigma * (l / fmax(g - g_tresh, 0));
  }

  return x;
}

bool ATFG::GenerateHistogram()
{
  if (m_scalar_histogram)
    delete m_scalar_histogram;

  unsigned int hl_size = m_max_laplacian - m_min_laplacian + 1;
  unsigned int hg_size = m_max_gradient + 1;
  
  // Histogram memory allocation and initialization
  m_scalar_histogram = new unsigned char**[MAX_V];
  m_gradient_qtd = new unsigned int*[MAX_V];
  m_average_laplacian = new float*[MAX_V];
  if (!m_scalar_histogram || !m_gradient_qtd || !m_average_laplacian)
  {
    printf("Insuficient memory to generate histogram!\n");
    return false;
  }

  for (unsigned long i = 0; i < MAX_V; ++i)
  {
    m_average_gradient[i] = 0;
    m_value_qtd[i] = 0;

    m_scalar_histogram[i] = new unsigned char*[hg_size];
    m_gradient_qtd[i] = new unsigned int[hg_size];
    m_average_laplacian[i] = new float[hg_size];
    if (!m_scalar_histogram[i] || !m_gradient_qtd[i] || !m_average_laplacian[i]) {
      printf("Insuficient memory to generate histogram!\n");
      return false;
    }
    
    for (unsigned long j = 0; j < hg_size; ++j)
    {
      m_gradient_qtd[i][j] = 0;
      m_average_laplacian[i][j] = -FLT_MAX;
      m_scalar_histogram[i][j] = new unsigned char[hl_size];
      if (!m_scalar_histogram[i][j]) {
        printf("Insuficient memory to generate histogram!\n");
        return false;
      }
      for (unsigned int k = 0; k < hl_size; ++k)
        m_scalar_histogram[i][j][k] = 0.0f;
    }
  }


  // Fill Histogram 
  for (int x = 0; x < m_width; x++)
  {
    for (int y = 0; y < m_height; y++)
    {
      for (int z = 0; z < m_depth; z++)
      {
        unsigned int vol_id = x + (y * m_width) + (z * m_width * m_height);
        unsigned char v = (unsigned char)m_volume->GetValue(vol_id);
        unsigned long g = (unsigned long)m_scalar_gradient[vol_id];
        long l = (long)m_scalar_laplacian[vol_id];

        if (v < 0.0f || v > 255.0f)
          continue;

        m_average_gradient[v] += g;
        ++m_value_qtd[v];

        if (m_average_laplacian[v][g] == -FLT_MAX)
          m_average_laplacian[v][g] = l;
        else
          m_average_laplacian[v][g] += l;
        ++m_gradient_qtd[v][g];

        if (m_scalar_histogram[v][g][l - m_min_laplacian] < MAX_V - 1)
          ++m_scalar_histogram[v][g][l - m_min_laplacian];
      }
    }
  }

  for (size_t i = 0; i < MAX_V; i++)
  {
    if (m_value_qtd[i] > 0)
      m_average_gradient[i] /= m_value_qtd[i];

    for (unsigned long j = 0; j < hg_size; j++)
    {
      if (m_gradient_qtd[i][j] > 0)
        m_average_laplacian[i][j] /= m_gradient_qtd[i][j];
    }
  }

  GenerateValueGradientSlice();
  GenerateValueLaplaceSlice();

  return true;
}

void ATFG::GenerateValueGradientSlice()
{
  FILE* fp;
  fopen_s(&fp, "Histogram Gradient Slice.pgm", "wb");
  fprintf(fp, "P2\n%d %d\n255\n", MAX_V, m_max_gradient + 1);

  int d = 0;
  for (int j = m_max_gradient; j >= 0; j--)
  {
    for (int i = 0; i < MAX_V; ++i)
    {
      unsigned char sum = 0;
      for (int k = 0; k < m_max_laplacian - m_min_laplacian + 1; k++)
      {
        sum += m_scalar_histogram[i][j][k];
      }
      
      fprintf(fp, "%d ", 255 - sum);
    }
    fprintf(fp, "\n");
  }
  fclose(fp);
}

void ATFG::GenerateValueLaplaceSlice()
{
  FILE* fp;
  fopen_s(&fp, "Histogram Laplace Slice.pgm", "wb");
  fprintf(fp, "P2\n%d %d\n255\n", MAX_V, m_max_laplacian - m_min_laplacian + 1);

  int d = 0;
  for (int k = m_max_laplacian - m_min_laplacian; k >= 0; --k)
  {
    for (int i = 0; i < MAX_V; ++i)
    {
      unsigned char sum = 0;
      for (int j = 0; j < m_max_gradient; ++j)
      {
        sum += m_scalar_histogram[i][j][k];
      }

      fprintf(fp, "%d ", 255 - sum);
    }
    fprintf(fp, "\n");
  }
  fclose(fp);
}

void ATFG::GenerateTransferFunction()
{
  std::ofstream state_file;
  state_file.open("../../Modelos//TransferFunctions//AutomaticTransferFunction.tf1d");

  state_file << "linear" << "\n";
  state_file << "0" << "\n";
  state_file << "9" << "\n";
  state_file << "1   1   1   0" << "\n";
  state_file << "1   0   0   32" << "\n";
  state_file << "0   1   0   64" << "\n";
  state_file << "0   0   1   96" << "\n";
  state_file << "0.5 0.5 0   128" << "\n";
  state_file << "0.5 0   0.5 160" << "\n";
  state_file << "0   0.5 0.5 192" << "\n";
  state_file << "0.3 0.3 0.3 224" << "\n";
  state_file << "0   0   0   255" << "\n";
  state_file << MAX_V << "\n";

  float* x = GetBoundaryDistancies();
  if (!x)
  {
    state_file.close();
    return;
  }

  float sigma = m_max_gradient / ((float)m_max_laplacian * SQRT_E);
  float base = sigma;
  float amax = 0.4f;

  for (size_t v = 0; v < MAX_V; ++v)
  {
    if (x[v] >= -base && x[v] <= base)
    {
      float a = 0.0f;
      if (x[v] >= 0.0)
        a = -(amax * x[v]) / base;
      else
        a = (amax * x[v]) / base;

      a += amax;
      state_file << a << "\t" << v << "\n";
    }
    else
      state_file << 0 << "\t" << v << "\n";
  }
  state_file.close();
  delete x;
}
