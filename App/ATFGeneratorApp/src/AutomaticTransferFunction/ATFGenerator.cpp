/// ATFGenerator.cpp
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#include "ATFGenerator.h"

#include <cstdlib>
#include <iostream>
#include <fstream>

#include <lqc/File/RAWLoader.h>
#include <math/MUtils.h>

#include "PGMFile.h"
#include "TransferFunction.h"


/// <summary>
/// Initializes a new instance of the 
/// <see cref="ATFGenerator" /> class.
/// </summary>
/// <param name="volume">The volume whose transfer 
/// function one wants to extract.</param>
ATFGenerator::ATFGenerator(vr::Volume* volume) : IATFGenerator(volume)
, m_scalar_gradient(NULL)
, m_scalar_laplacian(NULL)
, m_scalar_histogram(NULL)
, m_average_laplacian(NULL)
, m_transfer_function(NULL)
, m_max_global_gradient(-LONG_MAX)
, m_min_global_laplacian(LONG_MAX)
, m_max_global_laplacian(-LONG_MAX)
, m_initialized(false)
{
  for (size_t i = 0; i < MAX_V; ++i)
  {
    m_max_gradient[i] = -LONG_MAX;
    m_min_laplacian[i] = LONG_MAX;
    m_max_laplacian[i] = -LONG_MAX;
  }
}

/// <summary>
/// Finalizes an instance of the 
/// <see cref="ATFGenerator"/> class.
/// </summary>
ATFGenerator::~ATFGenerator()
{
  delete [] m_scalar_gradient;
  delete [] m_scalar_laplacian;

  if (!m_scalar_histogram)
    return;

  for (int i = 0; i < MAX_V; ++i)
  {
    delete [] m_average_laplacian[i];

    for (long j = 0; j < m_max_global_gradient + 1; ++j)
      delete [] m_scalar_histogram[i][j];
    
    delete[] m_scalar_histogram[i];
  }

  delete [] m_average_laplacian;
  delete [] m_scalar_histogram;
}

/// <summary>It does all the math necessary so information
/// can be extracted from it. The user will not be able to
/// get any kind of information without a successfull call
/// to this function. < / summary>
/// <returns>Returns true if the instance was correctly
/// initialized and false otherwise.</returns>
bool ATFGenerator::Init()
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

/// <summary>
/// Extract a transfer function from the volume, based in 
/// Kindlmann's paper.
/// </summary>
/// <returns>Returns true if the transfer function can be
/// generated. False, otherwise.</returns>
bool ATFGenerator::ExtractTransferFunction()
{
  if (!m_initialized)
    throw std::domain_error("Instance not initialized. Init must be called once!\n");
  
  std::string filename = m_volume->GetName();
  std::size_t init = filename.find_last_of("\\") + 1;
  std::size_t end = filename.find_first_of(".");
  filename = filename.substr(init, end - init);
  filename = "..\\..\\Modelos\\TransferFunctions\\AutomaticTransferFunction" + filename;

  delete m_transfer_function;
  m_transfer_function = new TransferFunction(filename.c_str());
  m_transfer_function->SetValueColor(0, 255, 255, 255);
  m_transfer_function->SetValueColor(32, 255, 0, 0);
  m_transfer_function->SetValueColor(64, 0, 255, 0);
  m_transfer_function->SetValueColor(96, 0, 0, 255);
  m_transfer_function->SetValueColor(128, 127, 127, 0);
  m_transfer_function->SetValueColor(160, 127, 0, 127);
  m_transfer_function->SetValueColor(192, 0, 127, 127);
  m_transfer_function->SetValueColor(224, 84, 84, 85);
  m_transfer_function->SetValueColor(255, 0, 0, 0);

  float* x = GetBoundaryDistancies();
  unsigned char* values = new unsigned char[MAX_V];
  float* sigmas = new float[MAX_V];
  
  if (!x || !values || !sigmas)
  {
    printf("Erro - Nao ha memoria suficiente para extrair a funcao de transferencia!\n");
    return false;
  }

  for (int i = 0; i < MAX_V; ++i)
  {
    values[i] = i;
    sigmas[i] = GetSigma(i);
  }

  m_transfer_function->SetClosestBoundaryDistances(values, x, sigmas, MAX_V);
  return m_transfer_function->Generate();
}

/// <summary>
/// Gets an aproximation of the voxel's gradient, using 
/// its first derivatives.
/// </summary>
/// <param name="x">The voxel's x component.</param>
/// <param name="y">The voxel's y component.</param>
/// <param name="z">The voxel's z component.</param>
/// <returns>Returns the float aproximated gradient.</returns>
float ATFGenerator::GetGradient(int x, int y, int z)
{
  if (!m_initialized)
    throw std::domain_error("Instance not initialized. Init must be called once!\n");

  assert(m_scalar_gradient);

  x = lqc::Clamp(x, 0, m_width - 1);
  y = lqc::Clamp(y, 0, m_height - 1);
  z = lqc::Clamp(z, 0, m_depth - 1);

  return m_scalar_gradient[x + (y * m_width) + (z * m_width * m_height)];
}

/// <summary>
/// Gets an aproximation of the voxel's laplacian, using 
/// its second derivatives.
/// </summary>
/// <param name="x">The voxel's x component.</param>
/// <param name="y">The voxel's y component.</param>
/// <param name="z">The voxel's z component.</param>
/// <returns>Returns the float aproximated laplacian.</returns>
float ATFGenerator::GetLaplacian(int x, int y, int z)
{
  if (!m_initialized)
    throw std::domain_error("Instance not initialized. Init must be called once!\n");

  assert(m_scalar_laplacian);

  x = lqc::Clamp(x, 0, m_width - 1);
  y = lqc::Clamp(y, 0, m_height - 1);
  z = lqc::Clamp(z, 0, m_depth - 1);

  return m_scalar_laplacian[x + (y * m_width) + (z * m_width * m_height)];
}

/// <summary>
/// Generates a PGM image file with a histogram slice.
/// The image is generated in the working directory and 
/// it's named "Histogram Slice v", where 'v' is the 
/// value input.
/// </summary>
/// <param name="v">The value whose slice it's desired.
/// The value must range from 0 to 255</param>
void ATFGenerator::GenerateHistogramSlice(unsigned int v)
{
  if (!m_initialized)
    throw std::domain_error("Instance not initialized. Init must be called once!\n");

  if (v >= MAX_V)
    throw std::domain_error("The value must range from 0 to 255!\n");

  char* filename = new char[19];
  sprintf(filename, "Histogram Slice %d", v);

  PGMFile pgmfile(filename, m_max_laplacian[v] - m_min_laplacian[v] + 1, m_max_gradient[v] + 1);

  if (!pgmfile.Open())
  {
    printf("Erro - O Histograma '%s' nao pode ser gerado!\n", filename);
    return;
  }
  delete [] filename;

  assert(m_scalar_histogram);

  for (int j = m_max_gradient[v]; j >= 0; j--)
  {
    for (int k = 0; k < m_max_laplacian[v] - m_min_laplacian[v] + 1; k++)
    {
      int h;
      if (m_scalar_histogram[v][j][k] >= 255)
        h = 255;
      else
        h = m_scalar_histogram[v][j][k];

      pgmfile.WriteByte(255 - h);
    }
    pgmfile.WriteEndLine();
  }
  pgmfile.Close();
}

/// <summary>
/// Generates the all the 256 histogram slices, trhought
/// calls to void GenerateHistogramSlice(int v).
/// </summary>
void ATFGenerator::GenerateHistogramSlices()
{
  for (int i = 0; i < MAX_V; ++i)
    GenerateHistogramSlice(i);
}

/// <summary>
/// Acumulates the gradient values of the 3D histogram, 
/// generating a 2D hitogram of laplacian values per 
/// intensity values. This 2D histogram is generated
/// as an PGM image file in the working directory, 
/// named "Gradient Summed Histogram".
/// </summary>
void ATFGenerator::GenerateGradientSummedHistogram()
{
  if (!m_initialized)
    throw std::domain_error("Instance not initialized. Init must be called once!\n");

  PGMFile pgmfile("Gradient Summed Histogram", MAX_V, m_max_global_gradient + 1);
  if (!pgmfile.Open())
    throw std::runtime_error("PGM file could not be created!");

  assert(m_scalar_histogram);

  for (long j = m_max_global_gradient; j >= 0; --j)
  {
    for (int i = 0; i < MAX_V; ++i)
    {
      unsigned char sum = 0;
      for (long k = 0; k < m_max_global_laplacian - m_min_global_laplacian + 1; ++k)
      {
        if ((int)sum + m_scalar_histogram[i][j][k] >= 255)
        {
          sum = 255;
          break;
        }
        else
          sum += m_scalar_histogram[i][j][k];
      }

      pgmfile.WriteByte(255 - sum);
    }
    pgmfile.WriteEndLine();
  }
  pgmfile.Close();
}

/// <summary>
/// Acumulates the laplacian values of the 3D histogram, 
/// generating a 2D hitogram of gradient values per 
/// intensity values. This 2D histogram is generated
/// as an PGM image file in the working directory, 
/// named "Laplacian Summed Histogram".
/// </summary>
void ATFGenerator::GenerateLaplacianSummedHistogram()
{
  if (!m_initialized)
    throw std::domain_error("Instance not initialized. Init must be called once!\n");

  PGMFile pgmfile("Laplacian Summed Histogram", MAX_V, m_max_global_laplacian - m_min_global_laplacian + 1);
  if (!pgmfile.Open())
    throw std::runtime_error("PGM file could not be created!");

  assert(m_scalar_histogram);

  for (long k = m_max_global_laplacian - m_min_global_laplacian; k >= 0; --k)
  {
    for (int i = 0; i < MAX_V; ++i)
    {
      unsigned char sum = 0;
      for (long j = 0; j < m_max_global_gradient; ++j)
      {
        if ((int)sum + m_scalar_histogram[i][j][k] >= 255)
        {
          sum = 255;
          break;
        }
        else
          sum += m_scalar_histogram[i][j][k];
      }

      pgmfile.WriteByte(255 - sum);
    }
    pgmfile.WriteEndLine();
  }
  pgmfile.Close();
}

/// <summary>
/// Gets the transfer function.
/// </summary>
/// <returns>Returns a pointer to the transfer function generated automatically.</returns>
ITransferFunction* ATFGenerator::GetTransferFunction()
{
  return m_transfer_function;
}

/// <summary>
/// Calculates an aproximation of the voxel's gradient, 
/// using its first derivatives.
/// </summary>
/// <param name="x">The voxel's x component.</param>
/// <param name="y">The voxel's y component.</param>
/// <param name="z">The voxel's z component.</param>
/// <returns>Returns the float aproximated gradient.</returns>
float ATFGenerator::CalculateGradient(int x, int y, int z)
{
  if (!m_volume)
    throw std::exception_ptr();

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

/// <summary>
/// Calculates an aproximation of the voxel's laplacian, 
/// using its second derivatives.
/// </summary>
/// <param name="x">The voxel's x component.</param>
/// <param name="y">The voxel's y component.</param>
/// <param name="z">The voxel's z component.</param>
/// <returns>Returns the float aproximated laplacian.</returns>
float ATFGenerator::CalculateLaplacian(int x, int y, int z)
{
  if (!m_volume)
    throw std::exception_ptr();

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

/// <summary>
/// Iterates over the volume, calculating the gradient 
/// and the laplacian values for each voxel.
/// </summary>
/// <returns>Returns true if all the memory needed 
/// was successfully created. Otherwise, it returns false.</returns>
bool ATFGenerator::CalculateVolumeDerivatives()
{
  assert(!m_scalar_gradient && !m_scalar_laplacian);

  unsigned int size = m_width * m_height * m_depth;
  if (size < (unsigned long)m_width * m_height * m_depth) {
    throw std::out_of_range("The volume dimensions are too big!\n");
  }

  if (size == 0) {
    throw std::out_of_range("The volume dimensions are not valid!\n");
  }

  m_scalar_gradient = new float[size];
  m_scalar_laplacian = new float[size];

  if (!m_scalar_gradient || !m_scalar_laplacian) {
    printf("Erro - Nao ha memoria suficiente para processar o volume!\n");
    return false;
  }

  for (int x = 0; x < m_width; x++)
  {
    bool x_border = (x == 0) | (x == (m_width - 1));
    for (int y = 0; y < m_height; y++)
    {
      bool y_border = (y == 0) | (y == (m_height - 1));
      for (int z = 0; z < m_depth; z++)
      {
        bool z_border = (z == 0) | (z == (m_depth - 1));
        if (x_border || y_border || z_border)
        {
          m_scalar_gradient[x + (y * m_width) + (z * m_width * m_height)] = 0;
          m_scalar_laplacian[x + (y * m_width) + (z * m_width * m_height)] = 0;
        }
        else
        {
          m_scalar_gradient[x + (y * m_width) + (z * m_width * m_height)] = CalculateGradient(x, y, z);
          m_scalar_laplacian[x + (y * m_width) + (z * m_width * m_height)] = CalculateLaplacian(x, y, z);
        }
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

  return true;
}

/// <summary>
/// Generates a 3D histogram which accumulates 
/// occurrences of value-gradient-laplacian triplets.
/// The volume derivatives must have been calculated 
/// before this call.
/// </summary>
/// <returns>Returns true if the histogram could be generated. 
/// False, otherwise.</returns>
bool ATFGenerator::GenerateHistogram()
{
  assert(!m_scalar_histogram && !m_average_laplacian);

  // Histogram memory allocation and initialization
  m_scalar_histogram = new unsigned char**[MAX_V];
  m_average_laplacian = new float*[MAX_V];
  if (!m_scalar_histogram || !m_average_laplacian)
  {
    printf("Erro - Nao ha memoria suficiente para gerar o histograma!\n");
    return false;
  }

  unsigned long hl_size = (unsigned long)m_max_global_laplacian - m_min_global_laplacian + 1;
  unsigned long hg_size = m_max_global_gradient + 1;
  for (int i = 0; i < MAX_V; ++i)
  {
    m_scalar_histogram[i] = new unsigned char*[hg_size];
    m_average_laplacian[i] = new float[hg_size];
    if (!m_scalar_histogram[i] || !m_average_laplacian[i]) {
      printf("Erro - Nao ha memoria suficiente para gerar o histograma!\n");
      return false;
    }

    for (unsigned long j = 0; j < hg_size; ++j)
    {
      m_average_laplacian[i][j] = -FLT_MAX;
      m_scalar_histogram[i][j] = new unsigned char[hl_size];
      if (!m_scalar_histogram[i][j]) {
        printf("Erro - Nao ha memoria suficiente para gerar o histograma!\n");
        return false;
      }
      for (unsigned long k = 0; k < hl_size; ++k)
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
    for (unsigned long j = 0; j < m_max_global_gradient + 1; ++j)
    {
      float wl = 0.0f;
      float l = 0.0f;
      for (unsigned long k = 0; k < m_max_global_laplacian - m_min_global_laplacian + 1; ++k)
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

/// <summary>
/// Gets, for each value, the distance to the closest 
/// boundary associated to it. This information is 
/// extracted from the summed voxel histogram.
/// </summary>
/// <returns>Returns a float array with the distances associated 
/// to all 256 values, ordered by value.</returns>
float* ATFGenerator::GetBoundaryDistancies()
{
  assert(m_scalar_histogram && m_average_laplacian);

  float* x = new float[MAX_V];
  if (!x)
    return NULL;

  for (int v = 0; v < MAX_V; ++v)
  {
    float sigma = GetSigma(v);
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

/// <summary>
/// Gets the sigma value of the closest boundary associated the specified value.
/// </summary>
/// <param name="v">The intendity value.</param>
/// <returns>Returns the float sigma value.</returns>
float ATFGenerator::GetSigma(unsigned char v)
{
  float sigma = 2 * SQRT_E * ((float)m_max_gradient[v] / (m_max_laplacian[v] - m_min_laplacian[v]));
  return sigma;
}
