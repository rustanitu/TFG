/// ATFGenerator.cpp
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#include "ATFGenerator.h"

#include <cstdlib>
#include <iostream>
#include <fstream>

#include <lqc/File/RAWLoader.h>
#include <math/MUtils.h>
#include <iup.h>
#include <iup_plot.h>

#include "PGMFile.h"
#include "Histogram.h"


#define ATFG_FULL_RANGE
#define ATFG_GAMA_CORRECTION 0.33f
#define MASK_SIZE 3

/// <summary>
/// Initializes a new instance of the 
/// <see cref="ATFGenerator" /> class.
/// </summary>
/// <param name="scalarfield">The scalarfield whose transfer 
/// function one wants to extract.</param>
ATFGenerator::ATFGenerator(vr::ScalarField* scalarfield) : IATFGenerator(scalarfield)
, m_scalar_gradient(NULL)
, m_scalar_laplacian(NULL)
, m_transfer_function(NULL)
, m_max_global_gradient(-FLT_MAX)
, m_min_global_laplacian(FLT_MAX)
, m_max_global_laplacian(-FLT_MAX)
, m_initialized(false)
, m_gtresh(0.0f)
, m_derivativeMask(MASK_SIZE)
, m_min_hist(0)
, m_main_plot(NULL)
, m_tf_plot(NULL)
, m_bx_plot(NULL)
, m_max_gradient(-FLT_MAX)
, m_min_gradient(FLT_MAX)
, m_max_laplacian(-FLT_MAX)
, m_min_laplacian(FLT_MAX)
{
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

void ATFGenerator::SetDefaultColor()
{
  m_transfer_function->AddRGBControlPoint(vr::TransferControlPoint(255 / 255.0, 255 / 255.0, 255 / 255.0, 0));
  m_transfer_function->AddRGBControlPoint(vr::TransferControlPoint(255 / 255.0, 0 / 255.0, 0 / 255.0, 32));
  m_transfer_function->AddRGBControlPoint(vr::TransferControlPoint(0 / 255.0, 255 / 255.0, 0 / 255.0, 64));
  m_transfer_function->AddRGBControlPoint(vr::TransferControlPoint(0 / 255.0, 0 / 255.0, 255 / 255.0, 96));
  m_transfer_function->AddRGBControlPoint(vr::TransferControlPoint(127 / 255.0, 127 / 255.0, 0 / 255.0, 128));
  m_transfer_function->AddRGBControlPoint(vr::TransferControlPoint(127 / 255.0, 0 / 255.0, 127 / 255.0, 160));
  m_transfer_function->AddRGBControlPoint(vr::TransferControlPoint(0 / 255.0, 127 / 255.0, 127 / 255.0, 192));
  m_transfer_function->AddRGBControlPoint(vr::TransferControlPoint(84 / 255.0, 84 / 255.0, 85 / 255.0, 224));
  m_transfer_function->AddRGBControlPoint(vr::TransferControlPoint(0 / 255.0, 0 / 255.0, 0 / 255.0, 255));
}

/// <summary>
/// Extract a transfer function from the scalarfield, based in 
/// Kindlmann's paper.
/// </summary>
/// <returns>Returns true if the transfer function can be
/// generated. False, otherwise.</returns>
bool ATFGenerator::ExtractTransferFunction()
{
  if (!m_initialized)
    throw std::domain_error("Instance not initialized. Init must be called once!\n");
  
  delete m_transfer_function;
  m_transfer_function = new vr::TransferFunction1D();
  //m_transfer_function->SetVolume(m_scalarfield);
  m_transfer_function->SetName(std::string("AutomaticTransferFunction"));
  m_transfer_function->SetTransferFunctionPlot(m_tf_plot);
  m_transfer_function->SetBoundaryFunctionPlot(m_bx_plot);
  SetDefaultColor();

  float* x = new float[ATFG_V_RANGE];
  unsigned char* v = new unsigned char[ATFG_V_RANGE];
  if (!x || !v)
  {
    printf("Erro - Nao ha memoria suficiente para extrair a funcao de transferencia!\n");
    return false;
  }

  UINT32 n_v;
  float sigma = GetBoundaryDistancies(x, v, &n_v);
  GenerateDataValuesFile(x, v, n_v);
  m_transfer_function->SetClosestBoundaryDistances(v, x, n_v);
  return true;
}

/// <summary>
/// Gets the voxel's value.
/// </summary>
/// <param name="x">The voxel's x component.</param>
/// <param name="y">The voxel's y component.</param>
/// <param name="z">The voxel's z component.</param>
/// <returns>Returns the float aproximated gradient.</returns>
int ATFGenerator::GetValue(int x, int y, int z)
{
	const UINT32 xt = std::max(0, std::min(x, (int) m_width - 1));
	const UINT32 yt = std::max(0, std::min(y, (int) m_height - 1));
	const UINT32 zt = std::max(0, std::min(z, (int) m_depth - 1));
  return m_scalarfield->GetValue(xt, yt, zt);
}

/// <summary>
/// Gets an aproximation of the voxel's gradient, using 
/// its first derivatives.
/// </summary>
/// <param name="x">The voxel's x component.</param>
/// <param name="y">The voxel's y component.</param>
/// <param name="z">The voxel's z component.</param>
/// <returns>Returns the float aproximated gradient.</returns>
float ATFGenerator::GetGradient(const UINT32& x, const UINT32& y, const UINT32& z)
{
  if (!m_initialized)
    throw std::domain_error("Instance not initialized. Init must be called once!\n");

  assert(m_scalar_gradient);

  UINT32 vx = lqc::Clamp(x, 0, m_width - 1);
  UINT32 vy = lqc::Clamp(y, 0, m_height - 1);
  UINT32 vz = lqc::Clamp(z, 0, m_depth - 1);

  return m_scalar_gradient[m_scalarfield->GetId(vx,vy,vz)];
}

/// <summary>
/// Gets an aproximation of the voxel's laplacian, using 
/// its second derivatives.
/// </summary>
/// <param name="x">The voxel's x component.</param>
/// <param name="y">The voxel's y component.</param>
/// <param name="z">The voxel's z component.</param>
/// <returns>Returns the float aproximated laplacian.</returns>
float ATFGenerator::GetLaplacian(const UINT32& x, const UINT32& y, const UINT32& z)
{
  if (!m_initialized)
    throw std::domain_error("Instance not initialized. Init must be called once!\n");

  assert(m_scalar_laplacian);

  UINT32 vx = lqc::Clamp(x, 0, m_width - 1);
  UINT32 vy = lqc::Clamp(y, 0, m_height - 1);
  UINT32 vz = lqc::Clamp(z, 0, m_depth - 1);

  return m_scalar_laplacian[m_scalarfield->GetId(vx,vy,vz)];
}

/// <summary>
/// Generates a PGM image file with a scalarfield slice.
/// The image is generated in the working directory and 
/// it's named "Volume Slice v", where 'v' is the 
/// value input.
/// </summary>
/// <param name="v">The value whose slice it's desired.</param>
void ATFGenerator::GenerateVolumeSlice(const UINT32& k)
{
  if (k >= m_depth)
    throw std::domain_error("The value must range from 0 to depth scalarfield!\n");

  char* filename = new char[50];
  sprintf(filename, "Volume\\Volume Slice %d", k);

  PGMFile pgmfile(filename, m_width, m_height);

  if (!pgmfile.Open())
  {
    printf("Erro - A fatia do scalarfield '%s' nao pode ser gerado!\n", filename);
    return;
  }
  delete[] filename;

  for (UINT32 j = m_height; j > 0; --j)
  {
    for (UINT32 i = 0; i < m_width; ++i)
    {
      pgmfile.WriteByte(m_scalarfield->GetValue(i,j-1,k));
    }
    pgmfile.WriteEndLine();
  }
  pgmfile.Close();
}

void ATFGenerator::GenerateGradientSlice(const UINT32& k)
{
  if (k >= m_depth)
    throw std::domain_error("The value must range from 0 to depth scalarfield!\n");

  char* filename = new char[50];
  sprintf(filename, "Histogram\\Gradient Slice %d", k);

  PGMFile pgmfile(filename, m_width, m_height);

  if (!pgmfile.Open()) {
    printf("Erro - A fatia do scalarfield '%s' nao pode ser gerado!\n", filename);
    return;
  }
  delete[] filename;

  for (UINT32 j = m_height; j > 0; --j) {
    for (UINT32 i = 0; i < m_width; ++i) {
      unsigned char v = 255;
      if (m_scalar_gradient[m_scalarfield->GetId(i, j-1, k)] < 256.0f)
        v = m_scalar_gradient[m_scalarfield->GetId(i, j-1, k)];
      pgmfile.WriteByte(v);
    }
    pgmfile.WriteEndLine();
  }
  pgmfile.Close();
}

void ATFGenerator::GenerateLaplacianSlice(const UINT32& k)
{
  if (k >= m_depth)
    throw std::domain_error("The value must range from 0 to depth scalarfield!\n");

  char* filename = new char[50];
  sprintf(filename, "Histogram\\Laplacian Slice %d", k);

  PGMFile pgmfile(filename, m_width, m_height);

  if (!pgmfile.Open()) {
    printf("Erro - A fatia do scalarfield '%s' nao pode ser gerado!\n", filename);
    return;
  }
  delete[] filename;

  for (UINT32 j = m_height; j > 0; --j) {
    for (UINT32 i = 0; i < m_width; ++i) {
      unsigned char v = 255;
      if (m_scalar_laplacian[m_scalarfield->GetId(i, j-1, k)] < 256.0f)
        v = m_scalar_laplacian[m_scalarfield->GetId(i, j-1, k)];
      pgmfile.WriteByte(v);
    }
    pgmfile.WriteEndLine();
  }
  pgmfile.Close();
}

/// <summary>
/// Generates the all the scalarfield slices, trhought
/// calls to void GenerateVolumeSlice(const UINT32& v).
/// </summary>
void ATFGenerator::GenerateVolumeSlices()
{
  for (UINT32 k = 0; k < m_depth; ++k)
    GenerateVolumeSlice(k);
}

void ATFGenerator::GenerateGradientSlices()
{
  for (UINT32 k = 0; k < m_depth; ++k)
    GenerateGradientSlice(k);
}

void ATFGenerator::GenerateLaplacianSlices()
{
  for (UINT32 k = 0; k < m_depth; ++k)
    GenerateLaplacianSlice(k);
}

/// <summary>
/// Generates a PGM image file with a histogram slice.
/// The image is generated in the working directory and 
/// it's named "Histogram Slice v", where 'v' is the 
/// value input.
/// </summary>
/// <param name="v">The value whose slice it's desired.
/// The value must range from 0 to 255</param>
bool ATFGenerator::GenerateHistogramSlice(const UINT32& v)
{
  //if (!m_initialized)
  //  throw std::domain_error("Instance not initialized. Init must be called once!\n");

  if (v >= ATFG_V_RANGE)
    throw std::domain_error("The value must range from 0 to 255!\n");

  char* filename = new char[50];
  sprintf(filename, "Histogram\\Histogram Slice %d", v);

  PGMFile pgmfile(filename, ATFG_V_RANGE, ATFG_V_RANGE);

  if (!pgmfile.Open())
  {
    printf("Erro - O Histograma '%s' nao pode ser gerado!\n", filename);
    return false;
  }
  delete [] filename;

  unsigned char histogram[ATFG_V_RANGE][ATFG_V_RANGE];

  for (UINT32 j = ATFG_V_MAX + 1; j > 0; j--)
  {
    for (UINT32 k = 0; k < ATFG_V_RANGE; k++)
    {
      UINT32 h = ATFG_V_MAX;
      if (m_scalar_histogram[v][j-1][k] >= 255)
        h = 0;
      else if (m_scalar_histogram[v][j-1][k] >= m_min_hist)
        h -= m_scalar_histogram[v][j-1][k];

      histogram[j-1][k] = h;
    }
  }

  dip::equalizeHistogram(&histogram[0][0], ATFG_V_RANGE, ATFG_V_RANGE, &histogram[0][0]);

  for (UINT32 j = ATFG_V_MAX + 1; j > 0; --j) {
    for (UINT32 i = 0; i < ATFG_V_RANGE; ++i) {
      unsigned char val = (unsigned char)(ATFG_V_MAX * pow(float(histogram[i][j-1]) / ATFG_V_MAX, ATFG_GAMA_CORRECTION));
      pgmfile.WriteByte(histogram[i][j-1]);
    }
    pgmfile.WriteEndLine();
  }
  pgmfile.Close();

  return true;
}

/// <summary>
/// Generates the all the 256 histogram slices, trhought
/// calls to void GenerateHistogramSlice(const UINT32& v).
/// </summary>
void ATFGenerator::GenerateHistogramSlices()
{
  for (UINT32 i = 0; i < ATFG_V_RANGE; ++i)
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

  unsigned char histogram[ATFG_V_RANGE][ATFG_V_RANGE];

  for (UINT32 j = ATFG_V_MAX + 1; j > 0; --j)
  {
    for (UINT32 i = 0; i < ATFG_V_RANGE; ++i)
    {
      int sum = ATFG_V_MAX;
      for (long k = 0; k < ATFG_V_RANGE; ++k) {
        sum -= m_scalar_histogram[i][j-1][k];
        if (sum < 0) {
          sum = 0;
          break;
        }
      }
      if (ATFG_V_MAX - sum > m_min_hist)
        histogram[i][j-1] = sum;
      else
        histogram[i][j-1] = ATFG_V_MAX;
    }
  }

  dip::equalizeHistogram(&histogram[0][0], ATFG_V_RANGE, ATFG_V_RANGE, &histogram[0][0]);

  for (UINT32 j = ATFG_V_MAX + 1; j > 0; --j) {
    for (UINT32 i = 0; i < ATFG_V_RANGE; ++i) {
      unsigned char val = (unsigned char)(ATFG_V_MAX * pow(float(histogram[i][j-1]) / ATFG_V_MAX, ATFG_GAMA_CORRECTION));
      pgmfile.WriteByte(val);
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

  unsigned char histogram[ATFG_V_RANGE][ATFG_V_RANGE];

  for (UINT32 k = ATFG_V_MAX + 1; k > 0; --k)
  {
    for (UINT32 i = 0; i < ATFG_V_RANGE; ++i)
    {
      int sum = ATFG_V_MAX;
      for (UINT32 j = 0; j < ATFG_V_RANGE; ++j)
      {
        sum -= m_scalar_histogram[i][j][k-1];
        if (sum < 0)
        {
          sum = 0;
          break;
        }
      }
      if (ATFG_V_MAX - sum > m_min_hist)
        histogram[i][k-1] = sum;
      else
        histogram[i][k-1] = ATFG_V_MAX;
    }
  }

  dip::equalizeHistogram(&histogram[0][0], ATFG_V_RANGE, ATFG_V_RANGE, &histogram[0][0]);

  for (UINT32 j = ATFG_V_MAX + 1; j > 0; --j) {
    for (UINT32 i = 0; i < ATFG_V_RANGE; ++i) {
      unsigned char val = (unsigned char)(ATFG_V_MAX * pow(float(histogram[i][j-1]) / ATFG_V_MAX, ATFG_GAMA_CORRECTION));
      pgmfile.WriteByte(val);
    }
    pgmfile.WriteEndLine();
  }
  pgmfile.Close();
}

void PrintFloat(std::ofstream& stream, float val)
{
  UINT32 ipart = abs(val);
  UINT32 fpart = abs((val - ipart) * 1000);

  if (val < 0)
    stream << "-";
  stream << ipart << "," << fpart << ";";
}

void ATFGenerator::GenerateDataValuesFile(float *x, unsigned char *v, const UINT32& n)
{
  if (!m_initialized)
    throw std::domain_error("Instance not initialized. Init must be called once!\n");

  IupSetAttribute(m_main_plot, "CLEAR", "YES");

  IupPlotBegin(m_main_plot, 0);
  for (UINT32 i = 0; i < ATFG_V_RANGE; i++)
    if (m_average_gradient[i] != -FLT_MAX)
      IupPlotAdd(m_main_plot, i, m_average_gradient[i]);
  IupPlotEnd(m_main_plot);
  IupSetAttribute(m_main_plot, "DS_NAME", "g(v)");
  IupSetAttribute(m_main_plot, "DS_COLOR", "0 0 128");

  IupPlotBegin(m_main_plot, 0);
  for (UINT32 i = 0; i < ATFG_V_RANGE; i++)
    if (m_average_laplacian[i] != -FLT_MAX)
      IupPlotAdd(m_main_plot, i, m_average_laplacian[i]);
  IupPlotEnd(m_main_plot);
  IupSetAttribute(m_main_plot, "DS_NAME", "h(v)");
  IupSetAttribute(m_main_plot, "DS_COLOR", "0 128 0");

  IupPlotBegin(m_main_plot, 0);
  IupPlotAdd(m_main_plot, 0, 0);
  IupPlotAdd(m_main_plot, 255, 0);
  IupPlotEnd(m_main_plot);
  IupSetAttribute(m_main_plot, "DS_NAME", "0");
  IupSetAttribute(m_main_plot, "DS_COLOR", "0 0 0");

  IupSetAttribute(m_main_plot, "REDRAW", "YES");
}

/// <summary>
/// Gets the transfer function.
/// </summary>
/// <returns>Returns a pointer to the transfer function generated automatically.</returns>
vr::TransferFunction* ATFGenerator::GetTransferFunction()
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
float ATFGenerator::CalculateGradient(const UINT32& x, const UINT32& y, const UINT32& z)
{
  if (!m_scalarfield)
    throw std::exception_ptr();

  float gx = GetValue(x + 1, y, z) - GetValue(x - 1, y, z);
  float gy = GetValue(x, y + 1, z) - GetValue(x, y - 1, z);
  float gz = GetValue(x, y, z + 1) - GetValue(x, y, z - 1);

  float g = sqrt(gx*gx + gy*gy + gz*gz);

  g = fmax(0.0f, g);
  m_max_global_gradient = fmax(m_max_global_gradient, g);

  return g;
}

float ATFGenerator::CalculateGradientByKernel(const UINT32& x, const UINT32& y, const UINT32& z)
{
  if (!m_scalarfield)
    throw std::exception_ptr();

  float g = 0.0f;
  float gx = 0.0f;
  float gy = 0.0f;
  float gz = 0.0f;

  UINT32 h = MASK_SIZE / 2;
  UINT32 xinit = x - h;
  UINT32 yinit = y - h;
  UINT32 zinit = z - h;
  for (UINT32 i = xinit; i <= x + h; ++i) {
    for (UINT32 j = yinit; j <= y + h; ++j) {
      for (UINT32 k = zinit; k <= z + h; ++k) {
        UINT32 v = GetValue(i, j, k);
        float pglx;
        float pgly;
        float pglz;
        m_derivativeMask.GetGradient(i - xinit, j - yinit, k - zinit, &pglx, &pgly, &pglz);
        gx += pglx * v;
        gy += pgly * v;
        gz += pglz * v;
      }
    }
  }

  g = sqrt(gx*gx + gy*gy + gz*gz);

  g = fmax(0.0f, g);
  if (g > m_max_global_gradient)
    m_max_global_gradient = g;

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
float ATFGenerator::CalculateLaplacian(const UINT32& x, const UINT32& y, const UINT32& z)
{
  if (!m_scalarfield)
    throw std::exception_ptr();

  int v = GetValue(x, y, z) * 2;
  float lx = GetValue(x + 1, y, z) - v + GetValue(x - 1, y, z);
  float ly = GetValue(x, y + 1, z) - v + GetValue(x, y - 1, z);
  float lz = GetValue(x, y, z + 1) - v + GetValue(x, y, z - 1);
  float l = lx + ly + lz;

  m_max_global_laplacian = fmax(m_max_global_laplacian, l);
  m_min_global_laplacian = fmin(m_min_global_laplacian, l);

  return l;
}

float ATFGenerator::CalculateLaplacianByKernel(const UINT32& x, const UINT32& y, const UINT32& z)
{
  if (!m_scalarfield)
    throw std::exception_ptr();

  float l = 0.0f;
  float lx = 0.0f;
  float ly = 0.0f;
  float lz = 0.0f;

  UINT32 h = MASK_SIZE / 2;
  UINT32 xinit = x - h;
  UINT32 yinit = y - h;
  UINT32 zinit = z - h;
  for (UINT32 i = xinit; i <= x + h; ++i) {
    for (UINT32 j = yinit; j <= y + h; ++j) {
      for (UINT32 k = zinit; k <= z + h; ++k) {
        UINT32 v = GetValue(i, j, k);
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

  if (l > m_max_global_laplacian)
    m_max_global_laplacian = l;
  if (l < m_min_global_laplacian)
    m_min_global_laplacian = l;

  return l;
}

/// <summary>
/// Iterates over the scalarfield, calculating the gradient 
/// and the laplacian values for each voxel.
/// </summary>
/// <returns>Returns true if all the memory needed 
/// was successfully created. Otherwise, it returns false.</returns>
bool ATFGenerator::CalculateVolumeDerivatives()
{
  assert(!m_scalar_gradient && !m_scalar_laplacian);

  UINT32 size = m_width * m_height * m_depth;
  if (size < (unsigned long)m_width * m_height * m_depth) {
    throw std::out_of_range("The scalarfield dimensions are too big!\n");
  }

  if (size == 0) {
    throw std::out_of_range("The scalarfield dimensions are not valid!\n");
  }

  delete [] m_scalar_gradient;
  delete [] m_scalar_laplacian;
  m_scalar_gradient = new float[size];
  m_scalar_laplacian = new float[size];

  if (!m_scalar_gradient || !m_scalar_laplacian) {
    printf("Erro - Nao ha memoria suficiente para processar o scalarfield!\n");
    return false;
  }

  for (UINT32 x = 0; x < m_width; ++x)
  {
    for (UINT32 y = 0; y < m_height; ++y)
    {
      for (UINT32 z = 0; z < m_depth; ++z)
      {
        UINT32 id = m_scalarfield->GetId(x, y, z);
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
/// The scalarfield derivatives must have been calculated 
/// before this call.
/// </summary>
/// <returns>Returns true if the histogram could be generated. 
/// False, otherwise.</returns>
bool ATFGenerator::GenerateHistogram()
{
  for (UINT32 i = 0; i < ATFG_V_RANGE; ++i)
  {
    for (UINT32 j = 0; j < ATFG_V_RANGE; ++j)
    {
      for (UINT32 k = 0; k < ATFG_V_RANGE; ++k)
      {
        m_scalar_histogram[i][j][k] = 0;
      }
    }
  }

#ifndef ATFG_FULL_RANGE
  m_max_global_gradient  *= 0.9;
  m_max_global_laplacian *= 0.9;
  m_min_global_laplacian *= 0.8;
#endif

  // Fill Histogram 
  for (UINT32 x = 0; x < m_width; x++)
  {
    for (UINT32 y = 0; y < m_height; y++)
    {
      for (UINT32 z = 0; z < m_depth; z++)
      {
        UINT32 vol_id = m_scalarfield->GetId(x,y,z);

#ifndef ATFG_FULL_RANGE
        if (m_scalar_gradient[vol_id] > m_max_global_gradient)
          continue;

        if (m_scalar_laplacian[vol_id] > m_max_global_laplacian || m_scalar_laplacian[vol_id] < m_min_global_laplacian)
          continue;
#endif

        unsigned char v = m_scalarfield->GetValue(vol_id);
        unsigned char g = (m_scalar_gradient[vol_id] / m_max_global_gradient) * ATFG_V_MAX;
        unsigned char l = ((m_scalar_laplacian[vol_id] - m_min_global_laplacian) / (m_max_global_laplacian - m_min_global_laplacian)) * ATFG_V_MAX;

        if (m_scalar_histogram[v][g][l] < ATFG_V_MAX)
          m_scalar_histogram[v][g][l]++;
      }
    }
  }
  
  m_max_gradient = -FLT_MAX;
  m_min_gradient = FLT_MAX;
  m_max_laplacian = -FLT_MAX;
  m_min_laplacian = FLT_MAX;

  // Calculate average laplacian and gradient
  for (UINT32 i = 0; i < ATFG_V_RANGE; ++i)
  {
    m_average_gradient[i] = -FLT_MAX;
    m_average_laplacian[i] = -FLT_MAX;
    UINT32 w = 0;
    float g = 0.0f;
    float h = 0.0f;
    for (UINT32 j = 0; j < ATFG_V_RANGE; ++j)
    {
      for (UINT32 k = 0; k < ATFG_V_RANGE; ++k)
      {
        if (m_scalar_histogram[i][j][k] > m_min_hist)
        {
          g += j;
          h += k;
          w++;
        }
      }
    }
    if (w > 0)
    {
      g /= w;
      g = m_max_global_gradient * g / ATFG_V_MAX;
      m_average_gradient[i] = g;
      m_max_gradient = fmax(m_max_gradient, g);
      m_min_gradient = fmin(m_min_gradient, g);

      h /= w;
      h = (m_max_global_laplacian - m_min_global_laplacian) * h / ATFG_V_MAX;
      h += m_min_global_laplacian;
      m_average_laplacian[i] = h;
      m_max_laplacian = fmax(m_max_laplacian, h);
      m_min_laplacian = fmin(m_min_laplacian, h);
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
float ATFGenerator::GetBoundaryDistancies(float * x, unsigned char *v, UINT32 *n)
{
  assert(m_scalar_histogram && x);

  if (m_max_laplacian == 0.0f)
    return 0.0f;

  float sigma = 2 * SQRT_E * m_max_gradient / (m_max_laplacian - m_min_laplacian);
  //float sigma = m_max_gradient / (m_max_laplacian * SQRT_E);

  UINT32 c = 0;
  for (UINT32 i = 0; i < ATFG_V_RANGE; ++i)
  {
    float g = m_average_gradient[i];
    float l = m_average_laplacian[i];
    if (g == -FLT_MAX || l == -FLT_MAX)
    {
      continue;
    }
    else
    {
      x[i] = -sigma * sigma * (l / fmax(g - m_gtresh, 0.000001));
    }

    v[c] = i;
    ++c;
  }

  *n = c;
  return sigma;
}
