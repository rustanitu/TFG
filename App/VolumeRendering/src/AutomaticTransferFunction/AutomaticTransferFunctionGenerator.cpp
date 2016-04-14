#include "AutomaticTransferFunctionGenerator.h"

#include <cstdlib>
#include <iostream>
#include <fstream>

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
   0,  1,  0,  0, -6,  0,  0,  1,  0,
   0,  0,  0,  0,  1,  0,  0,  0,  0
};


/************************/
/* Management Functions */
/************************/

ATFG::ATFG()
:m_width(0), m_height(0), m_depth(0), m_max_gradient(0), m_min_laplacian(0), m_max_laplacian(0),
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
  m_max_gradient = 0;
  m_max_laplacian = 0;
  m_min_laplacian = 0;

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
  GenerateHistogram();
}


/****************************************/
/* Transfer Function Generation Helpers */
/****************************************/

float ATFG::CalculateGradient(int x, int y, int z)
{
  int s = 0;
  float gx = 0;
  float gy = 0;
  float gz = 0;
  for (int i = x - 1; i <= x + 1; ++i)
  {
    for (int j = y - 1; j <= y + 1; ++j)
    {
      for (int k = z - 1; k <= z + 1; ++k)
      {
        float v = m_volume->GetValue(i, j, k);
        gx += SOBEL_X_MASK[s] * v;
        gy += SOBEL_Y_MASK[s] * v;
        gz += SOBEL_Z_MASK[s] * v;
        ++s;
      }
    }
  }

  gx /= 16.0f;
  gy /= 16.0f;
  gz /= 16.0f;

  float g = sqrt(gx*gx + gy*gy + gz*gz);
  if (g > m_max_gradient)
    m_max_gradient = g + 0.5f;

  return g;
}

float ATFG::CalculateLaplacian(int x, int y, int z)
{
  int s = 0;
  float lx = 0;
  float ly = 0;
  float lz = 0;
  for (int i = x - 1; i <= x + 1; ++i)
  {
    for (int j = y - 1; j <= y + 1; ++j)
    {
      for (int k = z - 1; k <= z + 1; ++k)
      {
        //float v = m_volume->GetValue(i, j, k);
        //lx += SOBEL_X_MASK[s] * v;
        //ly += SOBEL_Y_MASK[s] * v;
        //lz += SOBEL_Z_MASK[s] * v;
        //++s;
        lx += LAPLACE_MASK[s] * m_volume->GetValue(i, j, k);
        ++s;
      }
    }
  }

  //lx /= 16.0f;
  //ly /= 16.0f;
  //lz /= 16.0f;

  float l = lx;// sqrt(lx*lx + ly*ly + lz*lz);
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

  float x[MAX_V];

  for (size_t v = 0; v < MAX_V; ++v)
  {
    float g = m_average_gradient[v];
    float l = m_average_laplacian[v + MAX_V * (unsigned int)g];
    if (g <= 0.0f)
    {
      x[v] = 0.0f;
      continue;
    }

    x[v] = -(sigma * sigma * l) / g;
  }

  return x;
}

bool ATFG::GenerateHistogram()
{
  if (m_scalar_histogram)
    delete m_scalar_histogram;

  unsigned long size = MAX_V * (m_max_gradient + 1) * (m_max_laplacian - m_min_laplacian + 1);
  m_scalar_histogram = new unsigned char[size];
  if (!m_scalar_histogram)
  {
    printf("Insuficient memory to generate histogram!\n");
    return false;
  }

  for (unsigned int i = 0; i < size; ++i)
  {
    m_scalar_histogram[i] = 0;
  }

  size = MAX_V * (m_max_gradient + 1);
  m_gradient_qtd = new unsigned int[size];
  m_average_laplacian = new float[size];
  if (!m_gradient_qtd || !m_average_laplacian)
  {
    printf("Insuficient memory to generate histogram!\n");
    return false;
  }

  for (unsigned int i = 0; i < size; ++i)
  {
    m_gradient_qtd[i] = 0;
    m_average_laplacian[i] = -FLT_MAX;
  }

  for (size_t i = 0; i < MAX_V; i++)
  {
    m_average_gradient[i] = 0;
    m_value_qtd[i] = 0;
  }

  //alocate avereage h(g,v)

  for (int x = 0; x < m_width; x++)
  {
    for (int y = 0; y < m_height; y++)
    {
      for (int z = 0; z < m_depth; z++)
      {
        unsigned int vol_id = x + (y * m_width) + (z * m_width * m_height);
        float v = m_volume->GetValue(vol_id);
        float g = m_scalar_gradient[vol_id];
        float l = m_scalar_laplacian[vol_id];

        if (v < 0.0f || v > 255.0f)
          continue;

        unsigned int vid = v;
        m_average_gradient[vid] += g;
        ++m_value_qtd[vid];

        unsigned long lid = v + g * MAX_V;
        if (m_average_laplacian[lid] == -FLT_MAX)
          m_average_laplacian[lid] = l;
        else
          m_average_laplacian[lid] += l;
        ++m_gradient_qtd[lid];

        unsigned long hid = lid + ((l - m_min_laplacian) * MAX_V * (m_max_gradient + 1));
        if (m_scalar_histogram[hid] < MAX_V - 1)
          ++m_scalar_histogram[hid];
      }
    }
  }

  for (size_t i = 0; i < MAX_V; i++)
  {
    if (m_value_qtd[i] > 0)
      m_average_gradient[i] /= m_value_qtd[i];
  }

  for (unsigned long i = 0; i < size; i++)
  {
    if (m_gradient_qtd[i] > 0)
      m_average_laplacian[i] /= m_gradient_qtd[i];
  }

  return true;
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

  float base = 0.65f;
  float amax = 0.3f;

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
}
