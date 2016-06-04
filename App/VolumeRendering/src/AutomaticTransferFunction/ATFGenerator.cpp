#include "ATFGenerator.h"

#include <cstdlib>
#include <iostream>
#include <fstream>

#include <lqc/File/RAWLoader.h>
#include <math/MUtils.h>

#include "TransferFunction.h"


/*************************/
/* Constants Definitions */
/*************************/

#define SQRT_E 1.6487212707
#define MAX_V 256

/************************/
/* Management Functions */
/************************/

ATFGenerator::ATFGenerator()
:m_width(0), m_height(0), m_depth(0),
 m_scalar_gradient(NULL), m_scalar_laplacian(NULL), m_scalar_histogram(NULL), m_average_laplacian(NULL)
{
}


ATFGenerator::~ATFGenerator()
{
  CleanUp();
}

void ATFGenerator::Init()
{
  m_scalar_gradient = NULL;
  m_scalar_gradient = new float[m_width*m_height*m_depth];

  m_scalar_laplacian = NULL;
  m_scalar_laplacian = new float[m_width*m_height*m_depth];

  for (size_t i = 0; i < MAX_V; ++i) {
    m_max_gradient[i] = -LONG_MAX;
    m_min_laplacian[i] = LONG_MAX;
    m_max_laplacian[i] = -LONG_MAX;
  }

  for (unsigned int i = 0; i < m_width*m_height*m_depth; i++)
  {
    m_scalar_gradient[i] = 0.0f;
    m_scalar_laplacian[i] = 0.0f;
  }
}

void ATFGenerator::CleanUp()
{
  m_width = 0;
  m_height = 0;
  m_depth = 0;
  m_max_global_gradient = -LONG_MAX;
  m_min_global_laplacian = LONG_MAX;
  m_max_global_laplacian = -LONG_MAX;

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

float ATFGenerator::GetGradient(int x, int y, int z)
{
  x = lqc::Clamp(x, 0, m_width - 1);
  y = lqc::Clamp(y, 0, m_height - 1);
  z = lqc::Clamp(z, 0, m_depth - 1);

  return m_scalar_gradient[x + (y * m_width) + (z * m_width * m_height)];
}

float ATFGenerator::GetLaplacian(int x, int y, int z)
{
  x = lqc::Clamp(x, 0, m_width - 1);
  y = lqc::Clamp(y, 0, m_height - 1);
  z = lqc::Clamp(z, 0, m_depth - 1);

  return m_scalar_laplacian[x + (y * m_width) + (z * m_width * m_height)];
}

void ATFGenerator::SetVolume(vr::Volume* vol)
{
  CleanUp();

  m_volume = vol;
  m_width = vol->GetWidth();
  m_height = vol->GetHeight();
  m_depth = vol->GetDepth();

  Init();
  CalculateVolumeDerivatives();
  GenerateHistogram();
  //GenerateHistogramSlices();
  GenerateValueGradientSummedHistogram();
  GenerateValueLaplaceSummedHistogram();
  GetTransferFunction();
}


/****************************************/
/* Transfer Function Generation Helpers */
/****************************************/

float ATFGenerator::CalculateGradient(int x, int y, int z)
{
  int s = 0;
  float g = 0.0f;
  float gx = 0.0f;
  float gy = 0.0f;
  float gz = 0.0f;

  gx = 0.5f * (m_volume->GetValue(x + 1, y, z) - m_volume->GetValue(x - 1, y, z));
  gy = 0.5f * (m_volume->GetValue(x, y + 1, z) - m_volume->GetValue(x, y - 1, z));
  gz = 0.5f * (m_volume->GetValue(x, y, z + 1) - m_volume->GetValue(x, y, z - 1));

  g = sqrt(gx*gx + gy*gy + gz*gz);

  unsigned char v = m_volume->GetValue(x, y, z);
  g = fmax(0.0f, g);
  if (g > m_max_gradient[v])
    m_max_gradient[v] = g + 0.5f;
  if (g > m_max_global_gradient)
    m_max_global_gradient = g + 0.5f;

  return g;
}

float ATFGenerator::CalculateLaplacian(int x, int y, int z)
{
  int s = 0;
  float l = 0.0f;
  float lx = 0.0f;
  float ly = 0.0f;
  float lz = 0.0f;

  float g = 2 * m_volume->GetValue(x, y, z);
  lx = m_volume->GetValue(x + 1, y, z) - g + m_volume->GetValue(x - 1, y, z);
  ly = m_volume->GetValue(x, y + 1, z) - g + m_volume->GetValue(x, y - 1, z);
  lz = m_volume->GetValue(x, y, z + 1) - g + m_volume->GetValue(x, y, z - 1);

  l = lx + ly + lz;
  //l = sqrt(lx*lx + ly*ly + lz*lz);

  unsigned char v = m_volume->GetValue(x, y, z);
  if (l > m_max_laplacian[v])
    m_max_laplacian[v] = l + 0.5f;
  if (l < m_min_laplacian[v])
    m_min_laplacian[v] = l - 0.5f;

  if (l > m_max_global_laplacian)
    m_max_global_laplacian = l + 0.5f;
  if (l < m_min_global_laplacian)
    m_min_global_laplacian = l - 0.5f;

  return l;
}

void ATFGenerator::CalculateVolumeDerivatives()
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

  for (size_t i = 0; i < MAX_V; ++i) {
    if (m_max_gradient[i] == -LONG_MAX)
      m_max_gradient[i] = 0;
    if (m_max_laplacian[i] == -LONG_MAX)
      m_max_laplacian[i] = 0;
    if (m_min_laplacian[i] == LONG_MAX)
      m_min_laplacian[i] = 0;
  }
}

float* ATFGenerator::GetBoundaryDistancies()
{

  float* x = new float[MAX_V];

  for (size_t v = 0; v < MAX_V; ++v)
  {
    float sigma = Sigma(v);
    float g_tresh = 0.0f;// m_average_gradient[v] * 0.1f;
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

bool ATFGenerator::GenerateHistogram()
{
  if (m_scalar_histogram)
    delete m_scalar_histogram;
  
  // Histogram memory allocation and initialization
  m_scalar_histogram = new unsigned char**[MAX_V];
  m_average_laplacian = new float*[MAX_V];
  if (!m_scalar_histogram || !m_average_laplacian)
  {
    printf("Insuficient memory to generate histogram!\n");
    return false;
  }

  unsigned int hl_size = m_max_global_laplacian - m_min_global_laplacian + 1;
  unsigned int hg_size = m_max_global_gradient + 1;
  for (int i = 0; i < MAX_V; ++i)
  {
    m_scalar_histogram[i] = new unsigned char*[hg_size];
    m_average_laplacian[i] = new float[hg_size];
    if (!m_scalar_histogram[i] || !m_average_laplacian[i]) {
      printf("Insuficient memory to generate histogram!\n");
      return false;
    }

    for (long j = 0; j < hg_size; ++j)
    {
      m_average_laplacian[i][j] = -FLT_MAX;
      m_scalar_histogram[i][j] = new unsigned char[hl_size];
      if (!m_scalar_histogram[i][j]) {
        printf("Insuficient memory to generate histogram!\n");
        return false;
      }
      for (long k = 0; k < hl_size; ++k)
        m_scalar_histogram[i][j][k] = 0;
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

        if (m_scalar_histogram[v][g][l - m_min_global_laplacian] < MAX_V - 1)
          ++m_scalar_histogram[v][g][l - m_min_global_laplacian];
      }
    }
  }

  for (int i = 0; i < MAX_V; ++i)
  {
    float wg = 0.0f;
    float g = 0.0f;
    for (long j = 0; j < m_max_global_gradient + 1; ++j)
    {
      float wl = 0.0f;
      float l = 0.0f;
      for (long k = 0; k < m_max_global_laplacian - m_min_global_laplacian + 1; ++k)
      {
        float w = m_scalar_histogram[i][j][k] / 255.0f;
        wl += w;
        g += w * j;
        l += w * k;
      }
      wg += wl;
      l /= wl;
      m_average_laplacian[i][j] = l;
    }

    g /= wg;
    m_average_gradient[i] = g;
  }

  return true;
}

float ATFGenerator::Sigma(unsigned char v)
{
  float sigma = 2 * SQRT_E * ((float)m_max_gradient[v] / (m_max_laplacian[v] - m_min_laplacian[v]));
  return sigma;
}

void ATFGenerator::GenerateHistogramSlices()
{
  for (int i = 0; i < MAX_V; ++i)
    GenerateHistogramSlice(i);
}

void ATFGenerator::GenerateHistogramSlice(int v)
{
  FILE* fp;
  char filename[24];
  sprintf(filename, "Histogram Slice %d.pgm", v);
  fopen_s(&fp, filename, "wb");
  fprintf(fp, "P5\n%d %d\n255\n", m_max_laplacian[v] - m_min_laplacian[v] + 1, m_max_gradient[v] + 1);

  for (int j = m_max_gradient[v]; j >= 0; j--)
  {
    for (int k = 0; k < m_max_laplacian[v] - m_min_laplacian[v] + 1; k++)
    {
      int h;
      if (m_scalar_histogram[v][j][k] >= 255)
        h = 255;
      else
        h = m_scalar_histogram[v][j][k];
      
      fprintf(fp, "%d ", 255 - h);
    }
    fprintf(fp, "\n");
  }
  fclose(fp);
}

void ATFGenerator::GenerateValueGradientSummedHistogram()
{
  FILE* fp;
  fopen_s(&fp, "Gradient Summed Histogram.pgm", "wb");
  fprintf(fp, "P5\n%d %d\n255\n", MAX_V, m_max_global_gradient + 1);

  for (long j = m_max_global_gradient; j >= 0; --j) {
    for (int i = 0; i < MAX_V; ++i) {
      unsigned char sum = 0;
      for (long k = 0; k < m_max_global_laplacian - m_min_global_laplacian + 1; ++k) {
        if ((int)sum + m_scalar_histogram[i][j][k] >= 255)
        {
          sum = 255;
          break;
        }
        else
          sum += m_scalar_histogram[i][j][k];
      }

      fprintf(fp, "%d ", 255 - sum);
    }
    fprintf(fp, "\n");
  }
  fclose(fp);
}

void ATFGenerator::GenerateValueLaplaceSummedHistogram()
{
  FILE* fp;
  fopen_s(&fp, "Laplacian Summed Histogram.pgm", "wb");
  fprintf(fp, "P5\n%d %d\n255\n", MAX_V, m_max_global_laplacian - m_min_global_laplacian + 1);

  for (long k = m_max_global_laplacian - m_min_global_laplacian; k >= 0; --k) {
    for (int i = 0; i < MAX_V; ++i) {
      unsigned char sum = 0;
      for (long j = 0; j < m_max_global_gradient; ++j) {
        if ((int)sum + m_scalar_histogram[i][j][k] >= 255)
        {
          sum = 255;
          break;
        }
        else
          sum += m_scalar_histogram[i][j][k];
      }

      fprintf(fp, "%d ", 255 - sum);
    }
    fprintf(fp, "\n");
  }
  fclose(fp);
}

ITransferFunction* ATFGenerator::GetTransferFunction()
{
  std::string filename = m_volume->GetName();
  std::size_t init = filename.find_last_of("\\") + 1;
  std::size_t end = filename.find_first_of(".");
  filename = filename.substr(init, end - init);
  filename = "..\\..\\Modelos\\TransferFunctions\\AutomaticTransferFunction" + filename + ".tf1d";

  TransferFunction* ft = new TransferFunction();
  ft->SetValueColor(0, 255, 255, 255);
  ft->SetValueColor(32, 255, 0, 0);
  ft->SetValueColor(64, 0, 255, 0);
  ft->SetValueColor(96, 0, 0, 255);
  ft->SetValueColor(128, 127, 127, 0);
  ft->SetValueColor(160, 127, 0, 127);
  ft->SetValueColor(192, 0, 127, 127);
  ft->SetValueColor(224, 84, 84, 85);
  ft->SetValueColor(255, 0, 0, 0);

  float* x = GetBoundaryDistancies();
  unsigned char values[MAX_V];
  float sigmas[MAX_V];

  for (int i = 0; i < MAX_V; ++i)
  {
    values[i] = i;
    sigmas[i] = Sigma(i);
  }

  ft->SetClosestBoundaryDistances(values, x, sigmas, MAX_V);
  ft->Generate(filename.c_str());

  delete x;
  return ft;
}
