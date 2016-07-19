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
, m_transfer_function(NULL)
, m_max_global_gradient(-LONG_MAX)
, m_min_global_laplacian(LONG_MAX)
, m_max_global_laplacian(-LONG_MAX)
, m_initialized(false)
, m_gt(0.0f)
{
  GetValue(-1, 0, 0);
  GetValue(0, -1, 0);
  GetValue(0, 0, -1);
  GetValue(m_width, 0, 0);
  GetValue(0, m_height, 0);
  GetValue(0, 0, m_depth);
}

/// <summary>
/// Finalizes an instance of the 
/// <see cref="ATFGenerator"/> class.
/// </summary>
ATFGenerator::~ATFGenerator()
{
  delete m_transfer_function;
  delete [] m_scalar_gradient;
  delete [] m_scalar_laplacian;
}

/// <summary>It does all the math necessary so information
/// can be extracted from it. The user will not be able to
/// get any kind of information without a successfull call
/// to this function. </summary>
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

  float* x = new float[ATFG_V_RANGE];
  if (!x)
    return false;

  float sigma = GetBoundaryDistancies(x, m_gt);
  unsigned char* values = new unsigned char[ATFG_V_RANGE];
  
  if (!x || !values)
  {
    printf("Erro - Nao ha memoria suficiente para extrair a funcao de transferencia!\n");
    return false;
  }

  for (int i = 0; i < ATFG_V_RANGE; ++i)
  {
    values[i] = i;
  }

  m_transfer_function->SetClosestBoundaryDistances(values, x, sigma, ATFG_V_RANGE);
  return true;
}

/// <summary>
/// Gets the voxel's value.
/// </summary>
/// <param name="x">The voxel's x component.</param>
/// <param name="y">The voxel's y component.</param>
/// <param name="z">The voxel's z component.</param>
/// <returns>Returns the float aproximated gradient.</returns>
float ATFGenerator::GetValue(int x, int y, int z)
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

  return m_volume->GetValue(x, y, z);
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

  return m_scalar_gradient[GetId(x,y,z)];
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

  return m_scalar_laplacian[GetId(x,y,z)];
}

/// <summary>
/// Generates a PGM image file with a volume slice.
/// The image is generated in the working directory and 
/// it's named "Volume Slice v", where 'v' is the 
/// value input.
/// </summary>
/// <param name="v">The value whose slice it's desired.</param>
void ATFGenerator::GenerateVolumeSlice(unsigned int k)
{
  if (k >= m_depth)
    throw std::domain_error("The value must range from 0 to depth volume!\n");

  char* filename = new char[50];
  sprintf(filename, "Volume\\Volume Slice %d", k);

  PGMFile pgmfile(filename, m_width, m_height);

  if (!pgmfile.Open())
  {
    printf("Erro - A fatia do volume '%s' nao pode ser gerado!\n", filename);
    return;
  }
  //delete[] filename;

  for (int j = m_height - 1; j >= 0; --j)
  {
    for (int i = 0; i < m_width; ++i)
    {
      pgmfile.WriteByte(m_volume->GetValue(i,j,k));
    }
    pgmfile.WriteEndLine();
  }
  pgmfile.Close();
}

/// <summary>
/// Generates the all the volume slices, trhought
/// calls to void GenerateVolumeSlice(int v).
/// </summary>
void ATFGenerator::GenerateVolumeSlices()
{
  for (int k = 0; k < m_depth; ++k)
    GenerateVolumeSlice(k);
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

  if (v >= ATFG_V_RANGE)
    throw std::domain_error("The value must range from 0 to 255!\n");

  char* filename = new char[50];
  sprintf(filename, "Histogram\\Histogram Slice %d", v);

  PGMFile pgmfile(filename, ATFG_V_RANGE, ATFG_V_RANGE);

  if (!pgmfile.Open())
  {
    printf("Erro - O Histograma '%s' nao pode ser gerado!\n", filename);
    return;
  }
  //delete [] filename;

  for (int j = UCHAR_MAX; j >= 0; j--)
  {
    for (int k = 0; k < ATFG_V_RANGE; k++)
    {
      int h = UCHAR_MAX;
      if (m_scalar_histogram[v][j][k] >= 255)
        h = 0;
      else
        h -= m_scalar_histogram[v][j][k];

      pgmfile.WriteByte(h);
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
  for (int i = 0; i < ATFG_V_RANGE; ++i)
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

  PGMFile pgmfile("Gradient Summed Histogram", ATFG_V_RANGE, ATFG_V_RANGE);
  if (!pgmfile.Open())
    throw std::runtime_error("PGM file could not be created!");

  unsigned char min = 100;
  float a = float(UCHAR_MAX - min) / UCHAR_MAX;

  for (int j = UCHAR_MAX; j >= 0; --j)
  {
    for (int i = 0; i < ATFG_V_RANGE; ++i)
    {
      unsigned char sum = 0;
      for (long k = 0; k < ATFG_V_RANGE; ++k)
      {
        if ((int)sum + m_scalar_histogram[i][j][k] >= UCHAR_MAX)
        {
          sum = UCHAR_MAX;
          break;
        }
        else
          sum += m_scalar_histogram[i][j][k];
      }

      sum = pow(float(sum) / UCHAR_MAX, 0.125f) * UCHAR_MAX;

      //if (sum > 0)
      //{
      //  sum *= a;
      //  sum += min;
      //}

      pgmfile.WriteByte(UCHAR_MAX - sum);
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

  PGMFile pgmfile("Laplacian Summed Histogram", ATFG_V_RANGE, ATFG_V_RANGE);
  if (!pgmfile.Open())
    throw std::runtime_error("PGM file could not be created!");

  for (long k = UCHAR_MAX; k >= 0; --k)
  {
    for (int i = 0; i < ATFG_V_RANGE; ++i)
    {
      unsigned char sum = UCHAR_MAX;
      for (long j = 0; j < ATFG_V_RANGE; ++j)
      {
        if ((int)sum - m_scalar_histogram[i][j][k] <= 0)
        {
          sum = 0;
          break;
        }
        else
          sum -= m_scalar_histogram[i][j][k];
      }

      pgmfile.WriteByte(sum);
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

  float g = 0.0f;
  float gx = 0.0f;
  float gy = 0.0f;
  float gz = 0.0f;

  gx = 0.5f * (GetValue(x + 1, y, z) - GetValue(x - 1, y, z));
  gy = 0.5f * (GetValue(x, y + 1, z) - GetValue(x, y - 1, z));
  gz = 0.5f * (GetValue(x, y, z + 1) - GetValue(x, y, z - 1));
  g = sqrt(gx*gx + gy*gy + gz*gz);

  g = fmax(0.0f, g);
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

  float l = 0.0f;
  float lx = 0.0f;
  float ly = 0.0f;
  float lz = 0.0f;

  float g = 2 * GetValue(x, y, z);
  lx = GetValue(x + 1, y, z) - g + GetValue(x - 1, y, z);
  ly = GetValue(x, y + 1, z) - g + GetValue(x, y - 1, z);
  lz = GetValue(x, y, z + 1) - g + GetValue(x, y, z - 1);
  l = lx + ly + lz;

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

  for (int x = 0; x < m_width; ++x)
  {
    for (int y = 0; y < m_height; ++y)
    {
      for (int z = 0; z < m_depth; ++z)
      {
        unsigned int id = GetId(x, y, z);
        m_scalar_gradient[id] = CalculateGradient(x, y, z);
        m_scalar_laplacian[id] = CalculateLaplacian(x, y, z);
      }
    }
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
  for (int i = 0; i < ATFG_V_RANGE; ++i)
  {
    for (int j = 0; j < ATFG_V_RANGE; ++j)
    {
      for (int k = 0; k < ATFG_V_RANGE; ++k)
      {
        m_scalar_histogram[i][j][k] = 0;
      }
    }
  }

  m_max_global_gradient  *= 0.9;
  m_max_global_laplacian *= 0.9;
  m_min_global_laplacian *= 0.8;

  // Fill Histogram 
  for (int x = 0; x < m_width; x++)
  {
    for (int y = 0; y < m_height; y++)
    {
      for (int z = 0; z < m_depth; z++)
      {
        unsigned int vol_id = GetId(x,y,z);

        if (m_scalar_gradient[vol_id] > m_max_global_gradient)
          continue;

        if (m_scalar_laplacian[vol_id] > m_max_global_laplacian || m_scalar_laplacian[vol_id] < m_min_global_laplacian)
          continue;

        unsigned char v = m_volume->GetValue(vol_id);
        unsigned char g = (m_scalar_gradient[vol_id] / m_max_global_gradient) * UCHAR_MAX;
        unsigned char l = ((m_scalar_laplacian[vol_id] - m_min_global_laplacian) / (m_max_global_laplacian - m_min_global_laplacian)) * UCHAR_MAX;

        if (m_scalar_histogram[v][g][l] < UCHAR_MAX)
          m_scalar_histogram[v][g][l]++;
      }
    }
  }

  // Calculate average laplacian and gradient
  for (int i = 0; i < ATFG_V_RANGE; ++i)
  {
    int aw = 0.0f;
    float g = 0.0f;
    float h = 0.0f;
    for (unsigned int j = 0; j < ATFG_V_RANGE; ++j)
    {
      for (unsigned int k = 0; k < ATFG_V_RANGE; ++k)
      {
        unsigned int w = (m_scalar_histogram[i][j][k] ? 1 : 0);
        g += w * j;
        h += w * k;
        aw += w;
      }
    }

    if (aw < 1) {
      m_average_gradient[i] = 0;
      m_average_laplacian[i] = 0;
    }
    else
    {
      g /= aw;
      h /= aw;

      g = m_max_global_gradient * g / UCHAR_MAX;
      h = (m_max_global_laplacian - m_min_global_laplacian) * h / UCHAR_MAX;
      h += m_min_global_laplacian;

      m_average_gradient[i] = g;
      m_average_laplacian[i] = h;
    }
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
float ATFGenerator::GetBoundaryDistancies(float * x, float gt)
{
  assert(m_scalar_histogram && x);

  float max_gradient = 0.0f;
  float max_laplacian = 0.0f;
  for (int v = 0; v < ATFG_V_RANGE; ++v)
  {
    if (m_average_gradient[v] > max_gradient)
      max_gradient = m_average_gradient[v];
    if (m_average_laplacian[v] > max_laplacian)
      max_laplacian = m_average_laplacian[v];
  }

  if (max_laplacian == 0.0f)
    throw std::domain_error("Houve erro na analise dos dados. Laplaciano = 0.");
  
  float sigma = max_gradient / (max_laplacian * SQRT_E);
  float g_tresh = max_gradient * gt;

  for (int v = 0; v < ATFG_V_RANGE; ++v)
  {
    float g = m_average_gradient[v];
    float l = m_average_laplacian[v];
    if (g == 0.0f)
    {
      x[v] = FLT_MAX;
      continue;
    }

    x[v] = - sigma * sigma * (l / fmax(g - g_tresh, 0.000001));
  }

  return sigma;
}

unsigned int ATFGenerator::GetId(unsigned int x, unsigned int y, unsigned int z)
{
  return x + (y * m_width) + (z * m_width * m_height);
}