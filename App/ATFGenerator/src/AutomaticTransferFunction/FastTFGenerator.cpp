/// FastTFGenerator.cpp
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#include "FastTFGenerator.h"

#include <cstdlib>
#include <iostream>
#include <fstream>

#include <lqc/File/RAWLoader.h>
#include <math/MUtils.h>
#include <iup_plot.h>

#include "PGMFile.h"
#include "Histogram.h"


#define MASK_SIZE 3

FastTFGenerator::FastTFGenerator(vr::Volume* volume) : IATFGenerator(volume)
, m_derivativeMask(MASK_SIZE)
, m_initialized(false)
{
}

FastTFGenerator::~FastTFGenerator()
{
  delete m_transfer_function;
}

bool FastTFGenerator::Init()
{
  if (m_initialized)
    return true;

  if (!CalculateVolumeDerivatives())
    return false;

  if (!GenerateHistogram())
    return false;

  m_initialized = true;
  return true;
}

bool FastTFGenerator::ExtractTransferFunction()
{
  if (!m_initialized)
    throw std::domain_error("Instance not initialized. Init must be called once!\n");
  
  std::string filename = m_volume->GetName();
  std::size_t init = filename.find_last_of("\\") + 1;
  std::size_t end = filename.find_first_of(".");
  filename = filename.substr(init, end - init);
  filename = "..\\..\\Modelos\\TransferFunctions\\AutomaticTransferFunction" + filename;

  delete m_transfer_function;
  m_transfer_function = new vr::TransferFunction1D();
  m_transfer_function->SetTransferFunctionPlot(m_tf_plot);
  m_transfer_function->SetBoundaryFunctionPlot(m_bx_plot);

  float* x = new float[FAST_TFG_V_RANGE];
  unsigned char* v = new unsigned char[FAST_TFG_V_RANGE];
  if (!x || !v)
  {
    printf("Erro - Nao ha memoria suficiente para extrair a funcao de transferencia!\n");
    return false;
  }

  m_scalar_histogram_laplacian[0] = m_scalar_histogram_laplacian[1] - m_scalar_histogram_laplacian[0];
  for (int i = 1; i < FAST_TFG_V_MAX; ++i)
  {
    m_scalar_histogram_laplacian[i] = m_scalar_histogram[i + 1] - m_scalar_histogram[i - 1];
  }
  m_scalar_histogram_laplacian[FAST_TFG_V_MAX] = m_scalar_histogram_laplacian[FAST_TFG_V_MAX] - m_scalar_histogram_laplacian[FAST_TFG_V_MAX - 1];

  IupSetAttribute(m_bx_plot, "CLEAR", "YES");

  IupPlotBegin(m_bx_plot, 0);
  for (int i = 0; i < FAST_TFG_V_RANGE; i++)
    IupPlotAdd(m_bx_plot, i, m_scalar_histogram_laplacian[i]);
  IupPlotEnd(m_bx_plot);
  IupSetAttribute(m_bx_plot, "DS_NAME", "F'(T)");
  IupSetAttribute(m_bx_plot, "DS_COLOR", "0 0 128");

  IupSetAttribute(m_bx_plot, "REDRAW", "YES");

  //int n_v;
  //m_transfer_function->SetClosestBoundaryDistances(v, x, n_v);
  return true;
}

float FastTFGenerator::GetValue(int x, int y, int z)
{
  if (x < 0)
    x = -x;
  else if (x >= m_width)
    x = 2 * m_width - 1 - x;

  if (y < 0)
    y = -y;
  else if (y >= m_height)
    y = 2 * m_height - 1 - y;

  if (z < 0)
    z = -z;
  else if (z >= m_depth)
    z = 2 * m_depth - 1 - z;

  return m_volume->SampleVolume(x, y, z);
}

vr::TransferFunction* FastTFGenerator::GetTransferFunction()
{
  return m_transfer_function;
}

float FastTFGenerator::CalculateLaplacian(int x, int y, int z)
{
  if (!m_volume)
    throw std::exception_ptr();

  float l = 0.0f;
  float lx = 0.0f;
  float ly = 0.0f;
  float lz = 0.0f;

  int h = MASK_SIZE / 2;
  int xinit = x - h;
  int yinit = y - h;
  int zinit = z - h;
  for (int i = xinit; i <= x + h; ++i) {
    for (int j = yinit; j <= y + h; ++j) {
      for (int k = zinit; k <= z + h; ++k) {
        float v = GetValue(i, j, k);
        float pglx;
        float pgly;
        float pglz;
        m_derivativeMask.GetLaplacian(i - xinit, j - yinit, k - zinit, &pglx, &pgly, &pglz);
        lx += pglx * v;
        ly += pgly * v;
        lz += pglz * v;
      }
    }
  }

  l = lx + ly + lz;
  return l;
}

bool FastTFGenerator::CalculateVolumeDerivatives()
{
  assert(!m_scalar_laplacian);

  unsigned int size = m_width * m_height * m_depth;
  if (size < (unsigned long)m_width * m_height * m_depth) {
    throw std::out_of_range("The volume dimensions are too big!\n");
  }

  if (size == 0) {
    throw std::out_of_range("The volume dimensions are not valid!\n");
  }

  m_scalar_laplacian = new float[size];

  if (!m_scalar_laplacian) {
    printf("Erro - Nao ha memoria suficiente para processar o volume!\n");
    return false;
  }

  for (int x = 0; x < m_width; ++x)
  {
    for (int y = 0; y < m_height; ++y)
    {
      for (int z = 0; z < m_depth; ++z)
      {
        unsigned int id = GetId(x, y, z);
        m_scalar_laplacian[id] = CalculateLaplacian(x, y, z);
      }
    }
  }

  return true;
}

bool FastTFGenerator::GenerateHistogram()
{
  for (int i = 0; i < FAST_TFG_V_RANGE; ++i)
  {
    m_scalar_histogram[i] = 0.0f;
  }

  // Fill Histogram 
  for (int x = 0; x < m_width; x++)
  {
    for (int y = 0; y < m_height; y++)
    {
      for (int z = 0; z < m_depth; z++)
      {
        unsigned int vol_id = GetId(x,y,z);

        unsigned char v = m_volume->SampleVolume(vol_id);
        m_scalar_histogram[v] += m_scalar_laplacian[vol_id];
      }
    }
  }

  // Acumulate histogram
  for (int i = FAST_TFG_V_MAX - 1; i >= 0; --i)
  {
    m_scalar_histogram[i] += m_scalar_histogram[i+1];
  }

  for (int i = 0; i < FAST_TFG_V_RANGE; ++i) {
    m_scalar_histogram[i] = -m_scalar_histogram[i];
  }

  IupSetAttribute(m_main_plot, "CLEAR", "YES");

  IupPlotBegin(m_main_plot, 0);
  for (int i = 0; i < FAST_TFG_V_RANGE; i++)
    IupPlotAdd(m_main_plot, i, m_scalar_histogram[i]);
  IupPlotEnd(m_main_plot);
  IupSetAttribute(m_main_plot, "DS_NAME", "F(T)");
  IupSetAttribute(m_main_plot, "DS_COLOR", "0 0 128");

  IupSetAttribute(m_main_plot, "REDRAW", "YES");

  return true;
}

unsigned int FastTFGenerator::GetId(unsigned int x, unsigned int y, unsigned int z)
{
  return x + (y * m_width) + (z * m_width * m_height);
}