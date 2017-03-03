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
#include "../TankAPI/Tank.h"


#define ATFG_GAMA_CORRECTION 0.33f
//#define PLOT_STYLE "LINE"
#ifndef PLOT_STYLE
	#define PLOT_STYLE "MARK"
#endif

#define SMOOTH_CURVES

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
, m_initialized(false)
, m_gtresh(0.0f)
, m_min_hist(0)
, m_deriv_plot(NULL)
, m_dist_plot(NULL)
, m_max_average_gradient(-DBL_MAX)
, m_min_average_gradient(DBL_MAX)
, m_max_average_laplacian_1D(-DBL_MAX)
, m_min_average_laplacian_1D(DBL_MAX)
, m_max_average_laplacian_2D(-DBL_MAX)
, m_min_average_laplacian_2D(DBL_MAX)
, m_max_size(0)
, m_max_indexes(NULL)
, m_inflct_size(0)
, m_inflct_indexes(NULL)
, m_tf1d(true)
, m_tfmode_changed(false)
, m_average_hmap(NULL)
{
	printf("ATFGenerator criado.\n");
  m_average_hmap = new PredictionMap<double, DoubleCell>(ATFG_V_RANGE, ATFG_V_RANGE);
}

/// <summary>
/// Finalizes an instance of the 
/// <see cref="ATFGenerator"/> class.
/// </summary>
ATFGenerator::~ATFGenerator()
{
	delete m_transfer_function;
	delete[] m_scalar_gradient;
	delete[] m_scalar_laplacian;
  delete m_average_hmap;
	printf("ATFGenerator destruido.\n");
}

/// <summary>It does all the math necessary so information
/// can be extracted from it. The user will not be able to
/// get any kind of information without a successfull call
/// to this function. </summary>
/// <returns>Returns true if the instance was correctly
/// initialized and false otherwise.</returns>
bool ATFGenerator::Init()
{
	printf("--------------------------------------------------\n");
	printf("Inicializando ATFGenerator\n");
	if ( m_initialized )
		return true;

	if ( !CalculateVolumeDerivatives() )
		return false;

	if ( !GenerateHistogram() )
		return false;

  if (!EstimateAverageValues())
    return false;

	SmoothCurves();

	m_initialized = true;
	printf("ATFGenerator Inicializado.\n");
	printf("--------------------------------------------------\n");
	return true;
}

void ATFGenerator::SmoothCurves()
{
	const int times = 5;
	int size;
	double* values;
	int* indexes;

	GetValidValuesAndIndexes(m_average_laplacian, ATFG_V_RANGE, values, indexes, size);
	SmoothCurveWithGaussian(values, size, times);

	for ( int i = 0; i < size; ++i )
    m_average_laplacian[indexes[i]] = values[i];

	delete[] values;
	delete[] indexes;

  GetValidValuesAndIndexes(m_average_gradient, ATFG_V_RANGE, values, indexes, size);
  SmoothCurveWithGaussian(values, size, times);

  for (int i = 0; i < size; ++i)
    m_average_gradient[indexes[i]] = values[i];

  //*
  double min = DBL_MAX;
  int* min_indexes;
  size = GetMinPoints(values, indexes, size, min_indexes);
  for (int i = 0; i < size; ++i)
    min = fmin(min, m_average_gradient[min_indexes[i]]);
  
  m_gtresh = min;

  delete[] values;
  delete[] indexes;
  delete[] min_indexes;
  //*/
}

void ATFGenerator::SetDefaultColor()
{
  glm::vec3 colors[18] = 
  {
    glm::vec3(2u, 96u, 203u) / 255.0f,
    glm::vec3(0u, 127u, 206u) / 255.0f,
    glm::vec3(0u, 152u, 193u) / 255.0f,
    glm::vec3(11u, 171u, 152u) / 255.0f,
    glm::vec3(83u, 175u, 30u) / 255.0f,
    glm::vec3(129u, 178u, 0u) / 255.0f,
    glm::vec3(165u, 181u, 15u) / 255.0f,
    //glm::vec3(195u, 183u, 160u) / 255.0f,
    glm::vec3(216u, 184u, 22u) / 255.0f,
    glm::vec3(236u, 184u, 0u) / 255.0f,
    glm::vec3(255u, 182u, 0u) / 255.0f,
    glm::vec3(255u, 164u, 0u) / 255.0f,
    glm::vec3(255u, 149u, 0u) / 255.0f,
    glm::vec3(255u, 133u, 0u) / 255.0f,
    glm::vec3(255u, 112u, 0u) / 255.0f,
    glm::vec3(255u, 90u, 0u) / 255.0f,
    glm::vec3(255u, 66u, 11u) / 255.0f,
    glm::vec3(255u, 40u, 40u) / 255.0f,
    glm::vec3(255u, 0u, 0u) / 255.0f
  };
  double step = ATFG_V_RANGE / 18.f;
  int ncolors = 18;

  if (!m_tf1d)
  {
    vr::TransferFunction2D* tf = (vr::TransferFunction2D*)m_transfer_function;
    for (int i = 0; i < ncolors; ++i)
    {
      for (int j = i*step; j < (int)((i + 1)*step); ++j)
        for (int k = 0; k < ATFG_V_RANGE; ++k)
          tf->AddRGBControlPoint(colors[i], j, k);
    }
  }
  else
  {
    vr::TransferFunction1D* tf = (vr::TransferFunction1D*)m_transfer_function;
    for (int i = 0; i < ncolors; ++i) {
      for (int j = i*step; j < (int)((i + 1)*step); ++j)
        tf->AddRGBControlPoint(colors[i], j);
    }
  }
}

/// <summary>
/// Extract a transfer function from the scalarfield, based in 
/// Kindlmann's paper.
/// </summary>
/// <returns>Returns true if the transfer function can be
/// generated. False, otherwise.</returns>
bool ATFGenerator::ExtractTransferFunction()
{
	if ( !m_initialized )
		throw std::domain_error("Instance not initialized. Init must be called once!\n");

  if (!m_transfer_function || m_tfmode_changed)
	{
    m_tfmode_changed = false;
    delete m_transfer_function;

    if (m_tf1d)
		  m_transfer_function = new vr::TransferFunction1D();
    else
		  m_transfer_function = new vr::TransferFunction2D();

		m_transfer_function->SetName(std::string("AutomaticTransferFunction"));
		SetDefaultColor();
	}

	GenerateDataChart();

  PredictionMap<double, DoubleCell>* distmap = NULL;
  if (m_tf1d)
    distmap = GetBoundaryDistancies();
  else
    distmap = GetBoundaryDistancies2D();

  //distmap->PredictWithInverseDistanceWeighting(1.8f, 32);
  distmap->PredictWithRBF();
  m_transfer_function->SetClosestBoundaryDistances(distmap);
	
  return true;
}

/// <summary>
/// Gets the voxel's value.
/// </summary>
/// <param name="x">The voxel's x component.</param>
/// <param name="y">The voxel's y component.</param>
/// <param name="z">The voxel's z component.</param>
/// <returns>Returns the double aproximated gradient.</returns>
double ATFGenerator::GetValue(int x, int y, int z)
{
	return m_scalarfield->GetValue(x, y, z);
}

/// <summary>
/// Gets an aproximation of the voxel's gradient, using 
/// its first derivatives.
/// </summary>
/// <param name="x">The voxel's x component.</param>
/// <param name="y">The voxel's y component.</param>
/// <param name="z">The voxel's z component.</param>
/// <returns>Returns the double aproximated gradient.</returns>
double ATFGenerator::GetGradient(const UINT32& x, const UINT32& y, const UINT32& z)
{
	if ( !m_initialized )
		throw std::domain_error("Instance not initialized. Init must be called once!\n");

	assert(m_scalar_gradient);

	return m_scalar_gradient[m_scalarfield->GetId(x, y, z)];
}

/// <summary>
/// Gets an aproximation of the voxel's laplacian, using 
/// its second derivatives.
/// </summary>
/// <param name="x">The voxel's x component.</param>
/// <param name="y">The voxel's y component.</param>
/// <param name="z">The voxel's z component.</param>
/// <returns>Returns the double aproximated laplacian.</returns>
double ATFGenerator::GetLaplacian(const UINT32& x, const UINT32& y, const UINT32& z)
{
	if ( !m_initialized )
		throw std::domain_error("Instance not initialized. Init must be called once!\n");

	assert(m_scalar_laplacian);

	return m_scalar_laplacian[m_scalarfield->GetId(x, y, z)];
}

/// <summary>
/// Generates a PGM image file with a scalarfield slice.
/// The image is generated in the working directory and 
/// it's named "ScalarField Slice v", where 'v' is the 
/// value input.
/// </summary>
/// <param name="v">The value whose slice it's desired.</param>
void ATFGenerator::GenerateVolumeSlice(const UINT32& k)
{
	if ( k >= m_depth )
		throw std::domain_error("The value must range from 0 to depth scalarfield!\n");

	char* filename = new char[50];
	sprintf(filename, "Volume\\Volume Slice %d", k);

	PGMFile pgmfile(filename, m_width, m_height);

	if ( !pgmfile.Open() )
	{
		printf("Erro - A fatia do scalarfield '%s' nao pode ser gerado!\n", filename);
		return;
	}
	delete[] filename;

	for ( UINT32 j = m_height; j > 0; --j )
	{
		for ( UINT32 i = 0; i < m_width; ++i )
		{
			pgmfile.WriteByte(m_scalarfield->GetValue(i, j - 1, k));
		}
		pgmfile.WriteEndLine();
	}
	pgmfile.Close();
}

void ATFGenerator::GenerateGradientSlice(const UINT32& k)
{
	if ( k >= m_depth )
		throw std::domain_error("The value must range from 0 to depth scalarfield!\n");

	char* filename = new char[50];
	sprintf(filename, "Histogram\\Gradient Slice %d", k);

	PGMFile pgmfile(filename, m_width, m_height);

	if ( !pgmfile.Open() )
	{
		printf("Erro - A fatia do scalarfield '%s' nao pode ser gerado!\n", filename);
		return;
	}
	delete[] filename;

	for ( UINT32 j = m_height; j > 0; --j )
	{
		for ( UINT32 i = 0; i < m_width; ++i )
		{
			unsigned char v = 255;
			double g = m_scalarfield->GetScalarGradient(m_scalar_gradient[m_scalarfield->GetId(i, j - 1, k)], ATFG_V_MAX);
			if ( g < 256.0f )
				v = g;
			pgmfile.WriteByte(v);
		}
		pgmfile.WriteEndLine();
	}
	pgmfile.Close();
}

void ATFGenerator::GenerateLaplacianSlice(const UINT32& k)
{
	if ( k >= m_depth )
		throw std::domain_error("The value must range from 0 to depth scalarfield!\n");

	char* filename = new char[50];
	sprintf(filename, "Histogram\\Laplacian Slice %d", k);

	PGMFile pgmfile(filename, m_width, m_height);

	if ( !pgmfile.Open() )
	{
		printf("Erro - A fatia do scalarfield '%s' nao pode ser gerado!\n", filename);
		return;
	}
	delete[] filename;

	for ( UINT32 j = m_height; j > 0; --j )
	{
		for ( UINT32 i = 0; i < m_width; ++i )
		{
			unsigned char v = 255;
			double l = m_scalarfield->GetScalarLaplacian(m_scalar_laplacian[m_scalarfield->GetId(i, j - 1, k)], ATFG_V_MAX);
			if ( l < 256.0f )
				v = l;
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
	for ( UINT32 k = 0; k < m_depth; ++k )
		GenerateVolumeSlice(k);
}

void ATFGenerator::GenerateGradientSlices()
{
	for ( UINT32 k = 0; k < m_depth; ++k )
		GenerateGradientSlice(k);
}

void ATFGenerator::GenerateLaplacianSlices()
{
	for ( UINT32 k = 0; k < m_depth; ++k )
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

	if ( v >= ATFG_V_RANGE )
		throw std::domain_error("The value must range from 0 to 255!\n");

	char* filename = new char[50];
	sprintf(filename, "Histogram\\Histogram Slice %d", v);

	PGMFile pgmfile(filename, ATFG_V_RANGE, ATFG_V_RANGE);

	if ( !pgmfile.Open() )
	{
		printf("Erro - O Histograma '%s' nao pode ser gerado!\n", filename);
		return false;
	}
	delete[] filename;

	unsigned char histogram[ATFG_V_RANGE][ATFG_V_RANGE];

	for ( UINT32 j = ATFG_V_MAX + 1; j > 0; j-- )
	{
		for ( UINT32 k = 0; k < ATFG_V_RANGE; k++ )
		{
			UINT32 h = ATFG_V_MAX;
			if ( m_scalar_histogram[v][j - 1][k] >= 255 )
				h = 0;
			else if ( m_scalar_histogram[v][j - 1][k] >= m_min_hist )
				h -= m_scalar_histogram[v][j - 1][k];

			histogram[j - 1][k] = h;
		}
	}

	dip::equalizeHistogram(&histogram[0][0], ATFG_V_RANGE, ATFG_V_RANGE, &histogram[0][0]);

	for ( UINT32 j = ATFG_V_MAX + 1; j > 0; --j )
	{
		for ( UINT32 i = 0; i < ATFG_V_RANGE; ++i )
		{
			unsigned char val = (unsigned char) (ATFG_V_MAX * pow(double(histogram[i][j - 1]) / ATFG_V_MAX, ATFG_GAMA_CORRECTION));
			pgmfile.WriteByte(histogram[i][j - 1]);
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
	for ( UINT32 i = 0; i < ATFG_V_RANGE; ++i )
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
	if ( !m_initialized )
		throw std::domain_error("Instance not initialized. Init must be called once!\n");

	PGMFile pgmfile("Gradient Summed Histogram", ATFG_V_RANGE, ATFG_V_RANGE);
	if ( !pgmfile.Open() )
		throw std::runtime_error("PGM file could not be created!");

	unsigned char histogram[ATFG_V_RANGE][ATFG_V_RANGE];

	for ( UINT32 j = ATFG_V_MAX + 1; j > 0; --j )
	{
		for ( UINT32 i = 0; i < ATFG_V_RANGE; ++i )
		{
			int sum = ATFG_V_MAX;
			for ( long k = 0; k < ATFG_V_RANGE; ++k )
			{
				sum -= m_scalar_histogram[i][j - 1][k];
				if ( sum < 0 )
				{
					sum = 0;
					break;
				}
			}
			if ( ATFG_V_MAX - sum > m_min_hist )
				histogram[i][j - 1] = sum;
			else
				histogram[i][j - 1] = ATFG_V_MAX;
		}
	}

	dip::equalizeHistogram(&histogram[0][0], ATFG_V_RANGE, ATFG_V_RANGE, &histogram[0][0]);

	for ( UINT32 j = ATFG_V_MAX + 1; j > 0; --j )
	{
		for ( UINT32 i = 0; i < ATFG_V_RANGE; ++i )
		{
			unsigned char val = (unsigned char) (ATFG_V_MAX * pow(double(histogram[i][j - 1]) / ATFG_V_MAX, ATFG_GAMA_CORRECTION));
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
	if ( !m_initialized )
		throw std::domain_error("Instance not initialized. Init must be called once!\n");

	PGMFile pgmfile("Laplacian Summed Histogram", ATFG_V_RANGE, ATFG_V_RANGE);
	if ( !pgmfile.Open() )
		throw std::runtime_error("PGM file could not be created!");

	unsigned char histogram[ATFG_V_RANGE][ATFG_V_RANGE];

	for ( UINT32 k = ATFG_V_MAX + 1; k > 0; --k )
	{
		for ( UINT32 i = 0; i < ATFG_V_RANGE; ++i )
		{
			int sum = ATFG_V_MAX;
			for ( UINT32 j = 0; j < ATFG_V_RANGE; ++j )
			{
				sum -= m_scalar_histogram[i][j][k - 1];
				if ( sum < 0 )
				{
					sum = 0;
					break;
				}
			}
			if ( ATFG_V_MAX - sum > m_min_hist )
				histogram[i][k - 1] = sum;
			else
				histogram[i][k - 1] = ATFG_V_MAX;
		}
	}

	dip::equalizeHistogram(&histogram[0][0], ATFG_V_RANGE, ATFG_V_RANGE, &histogram[0][0]);

	for ( UINT32 j = ATFG_V_MAX + 1; j > 0; --j )
	{
		for ( UINT32 i = 0; i < ATFG_V_RANGE; ++i )
		{
			unsigned char val = (unsigned char) (ATFG_V_MAX * pow(double(histogram[i][j - 1]) / ATFG_V_MAX, ATFG_GAMA_CORRECTION));
			pgmfile.WriteByte(val);
		}
		pgmfile.WriteEndLine();
	}
	pgmfile.Close();
}

void ATFGenerator::GenerateDataChart()
{
	if ( !m_initialized )
		throw std::domain_error("Instance not initialized. Init must be called once!\n");

	IupSetAttribute(m_deriv_plot, "CLEAR", "YES");

	IupPlotBegin(m_deriv_plot, 0);
	IupPlotAdd(m_deriv_plot, 0, 0);
	IupPlotAdd(m_deriv_plot, ATFG_V_MAX, 0);
	IupPlotEnd(m_deriv_plot);
	IupSetAttribute(m_deriv_plot, "DS_NAME", "0");
	IupSetAttribute(m_deriv_plot, "DS_COLOR", "0 0 0");

	IupPlotBegin(m_deriv_plot, 0);
	IupPlotAdd(m_deriv_plot, 0, m_gtresh);
	IupPlotAdd(m_deriv_plot, 255, m_gtresh);
	IupPlotEnd(m_deriv_plot);
	IupSetAttribute(m_deriv_plot, "DS_MODE", "LINE");
	IupSetAttribute(m_deriv_plot, "DS_NAME", "Gtreh");
	IupSetAttribute(m_deriv_plot, "DS_COLOR", "0 0 0");

	IupPlotBegin(m_deriv_plot, 0);
	for ( UINT32 i = 0; i < ATFG_V_RANGE; i++ )
		if ( m_average_gradient[i] != -DBL_MAX )
			IupPlotAdd(m_deriv_plot, i, m_average_gradient[i]);
	IupPlotEnd(m_deriv_plot);
	IupSetAttribute(m_deriv_plot, "DS_MODE", PLOT_STYLE);
	IupSetAttribute(m_deriv_plot, "DS_MARKSTYLE", "CIRCLE");
	IupSetAttribute(m_deriv_plot, "DS_MARKSIZE", "3");
	IupSetAttribute(m_deriv_plot, "DS_NAME", "g(v)");
	IupSetAttribute(m_deriv_plot, "DS_COLOR", "0 0 128");

	IupPlotBegin(m_deriv_plot, 0);
	for ( UINT32 i = 0; i < ATFG_V_RANGE; i++ )
		if ( m_average_laplacian[i] != -DBL_MAX )
			IupPlotAdd(m_deriv_plot, i, m_average_laplacian[i]);
	IupPlotEnd(m_deriv_plot);
	IupSetAttribute(m_deriv_plot, "DS_MODE", PLOT_STYLE);
	IupSetAttribute(m_deriv_plot, "DS_MARKSTYLE", "CIRCLE");
	IupSetAttribute(m_deriv_plot, "DS_MARKSIZE", "3");
	IupSetAttribute(m_deriv_plot, "DS_NAME", "h(v)");
	IupSetAttribute(m_deriv_plot, "DS_COLOR", "0 128 0");

  /*******************************************************
  IupPlotBegin(m_deriv_plot, 0);
	for (UINT32 i = 0; i < m_max_size; i++)
		IupPlotAdd(m_deriv_plot, m_max_indexes[i], m_average_gradient[m_max_indexes[i]]);
	IupPlotEnd(m_deriv_plot);
	IupSetAttribute(m_deriv_plot, "DS_MODE", "MARK");
	IupSetAttribute(m_deriv_plot, "DS_MARKSTYLE", "CIRCLE");
	IupSetAttribute(m_deriv_plot, "DS_MARKSIZE", "3");
	IupSetAttribute(m_deriv_plot, "DS_NAME", "Max(v)");
	IupSetAttribute(m_deriv_plot, "DS_COLOR", "255 0 0");

	IupPlotBegin(m_deriv_plot, 0);
	for (UINT32 i = 0; i < m_inflct_size; i++)
		IupPlotAdd(m_deriv_plot, m_inflct_indexes[i], m_average_laplacian[m_inflct_indexes[i]]);
	IupPlotEnd(m_deriv_plot);
	IupSetAttribute(m_deriv_plot, "DS_MODE", "MARK");
	IupSetAttribute(m_deriv_plot, "DS_MARKSTYLE", "CIRCLE");
	IupSetAttribute(m_deriv_plot, "DS_MARKSIZE", "3");
	IupSetAttribute(m_deriv_plot, "DS_NAME", "Inflct(v)");
	IupSetAttribute(m_deriv_plot, "DS_COLOR", "255 0 255");
  //*******************************************************/

	IupSetAttribute(m_deriv_plot, "REDRAW", "YES");
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
/// Iterates over the scalarfield, calculating the gradient 
/// and the laplacian values for each voxel.
/// </summary>
/// <returns>Returns true if all the memory needed 
/// was successfully created. Otherwise, it returns false.</returns>
bool ATFGenerator::CalculateVolumeDerivatives()
{
	assert(!m_scalar_gradient && !m_scalar_laplacian);
	
	printf("--------------------------------------------------\n");
	printf("Calculando derivadas do campo escalar.\n");

	UINT32 size = m_width * m_height * m_depth;
	if ( size < (unsigned long) m_width * m_height * m_depth )
	{
		throw std::out_of_range("The scalarfield dimensions are too big!\n");
	}

	if ( size == 0 )
	{
		throw std::out_of_range("The scalarfield dimensions are not valid!\n");
	}

	delete[] m_scalar_gradient;
	delete[] m_scalar_laplacian;

	m_scalar_gradient = new double[size];
	m_scalar_laplacian = new double[size];

	if ( !m_scalar_gradient || !m_scalar_laplacian )
	{
		printf("Erro - Nao ha memoria suficiente para processar o scalarfield!\n");
		return false;
	}

#if 1
  for (UINT32 x = 0; x < m_width; ++x)
  {
    for (UINT32 y = 0; y < m_height; ++y)
    {
      for (UINT32 z = 0; z < m_depth; ++z)
      {
        UINT32 id = m_scalarfield->GetId(x, y, z);
        if (m_scalarfield->IsActive(x, y, z))
          m_scalar_gradient[id] = m_scalarfield->CalculateGradient(x, y, z);
        else
          m_scalar_gradient[id] = -DBL_MAX;
      }
    }
  }

  for (UINT32 x = 0; x < m_width; ++x)
  {
    for (UINT32 y = 0; y < m_height; ++y)
    {
      for (UINT32 z = 0; z < m_depth; ++z)
      {
        UINT32 id = m_scalarfield->GetId(x, y, z);
        if (m_scalarfield->IsActive(x, y, z))
          m_scalar_laplacian[id] = m_scalarfield->CalculateLaplacian(x, y, z);
        else
          m_scalar_laplacian[id] = -DBL_MAX;
      }
    }
  }
#else
	for ( UINT32 x = 0; x < m_width; ++x )
	{
		for ( UINT32 y = 0; y < m_height; ++y )
		{
			for ( UINT32 z = 0; z < m_depth; ++z )
			{
				UINT32 id = m_scalarfield->GetId(x, y, z);
        if (m_scalarfield->IsActive(x, y, z))
				  m_scalarfield->CalculateDerivatives(x, y, z, &m_scalar_gradient[id], &m_scalar_laplacian[id]);
        else
        {
          m_scalar_gradient[id] = -DBL_MAX;
          m_scalar_laplacian[id] = -DBL_MAX;
        }
			}
		}
	}
#endif

	printf("MaxGradient: %.2f\n", m_scalarfield->GetMaxGradient());
	printf("MinLaplacian: %.2f\n", m_scalarfield->GetMinLaplacian());
	printf("MaxLaplacian: %.2f\n", m_scalarfield->GetMaxLaplacian());

	printf("Derivadas calculadas.\n");
	printf("--------------------------------------------------\n");

	return true;
}

bool ATFGenerator::UpdateVolumeDerivatives()
{
  assert(m_scalar_gradient && m_scalar_laplacian);

  // Para remover esta verificacao basta escalar o "eixo correspondente da identidade".
  // Se não é tank não precisa da jacobiana inversa, mas ainda assim o gradiente é alterado com o escalar de um eixo.
  if (!m_scalarfield->IsTank())
    return true;

  Tank* tank = (Tank*)m_scalarfield;
  tank->ResetExtremeDerivatives();

  printf("--------------------------------------------------\n");
  printf("Atualizando derivadas do campo escalar.\n");

  for (UINT32 x = 0; x < m_width; ++x)
  {
    for (UINT32 y = 0; y < m_height; ++y)
    {
      for (UINT32 z = 0; z < m_depth; ++z)
      {
        UINT32 id = tank->GetId(x, y, z);
        if (tank->IsActive(x, y, z))
          tank->UpdateDerivatives(x, y, z, &m_scalar_gradient[id], &m_scalar_laplacian[id]);
        else
        {
          m_scalar_gradient[id] = -DBL_MAX;
          m_scalar_laplacian[id] = -DBL_MAX;
        }
      }
    }
  }

  printf("MaxGradient: %.2f\n", tank->GetMaxGradient());
  printf("MinLaplacian: %.2f\n", tank->GetMinLaplacian());
  printf("MaxLaplacian: %.2f\n", tank->GetMaxLaplacian());

  printf("Derivadas atualizadas.\n");
  printf("--------------------------------------------------\n");

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
	printf("--------------------------------------------------\n");
	printf("Gerando histograma.\n");

	for ( UINT32 i = 0; i < ATFG_V_RANGE; ++i )
	{
		for ( UINT32 j = 0; j < ATFG_V_RANGE; ++j )
		{
			for ( UINT32 k = 0; k < ATFG_V_RANGE; ++k )
			{
				m_scalar_histogram[i][j][k] = 0;
			}
		}
	}

	// Fill Histogram 
	for ( UINT32 x = 0; x < m_width; x++ )
	{
		for ( UINT32 y = 0; y < m_height; y++ )
		{
			for ( UINT32 z = 0; z < m_depth; z++ )
			{
				if ( !m_scalarfield->IsActive(x, y, z) )
					continue;

				UINT32 vol_id = m_scalarfield->GetId(x, y, z);

				unsigned char v = m_scalarfield->GetScalarValue(vol_id, ATFG_V_MAX);
				unsigned char g = m_scalarfield->GetScalarGradient(m_scalar_gradient[vol_id], ATFG_V_MAX);
        unsigned char l = 0;
        if (m_scalar_laplacian[vol_id] != -DBL_MAX)
          l = m_scalarfield->GetScalarLaplacian(m_scalar_laplacian[vol_id], ATFG_V_MAX);

				if ( m_scalar_histogram[v][g][l] < ATFG_V_MAX )
				{
					m_scalar_histogram[v][g][l]++;
				}
			}
		}
	}

	printf("Histograma gerado.\n");
	printf("--------------------------------------------------\n");

	return true;
}

bool ATFGenerator::EstimateAverageValues()
{
  printf("Extraindo derivadas medias, pelo histograma.\n");

  m_max_average_gradient = -DBL_MAX;
  m_min_average_gradient = DBL_MAX;
  m_max_average_laplacian_1D = -DBL_MAX;
  m_min_average_laplacian_1D = DBL_MAX;
  m_max_average_laplacian_2D = -DBL_MAX;
  m_min_average_laplacian_2D = DBL_MAX;

  if (!m_average_hmap->Init())
    return false;

  // Calculate average laplacian and gradient
  for (UINT32 i = 0; i < ATFG_V_RANGE; ++i) {
    m_average_gradient[i] = -DBL_MAX;
    m_average_laplacian[i] = -DBL_MAX;
    UINT32 w = 0;
    double g = 0.0f;
    double h = 0.0f;
    for (UINT32 j = 0; j < ATFG_V_RANGE; ++j)
    {
      //m_average_h[i][j] = -DBL_MAX;
      UINT32 hvg_sum = 0;
      double hvg = 0.0f;

      for (UINT32 k = 0; k < ATFG_V_RANGE; ++k) {
        if (m_scalar_histogram[i][j][k] > 0) {
          g += j;// * m_scalar_histogram[i][j][k];
          h += k;// * m_scalar_histogram[i][j][k];
          w++;// = m_scalar_histogram[i][j][k];
          hvg_sum++;
          hvg += k;
        }
      }

      if (hvg_sum > 0) {
        hvg /= hvg_sum;
        hvg = (m_scalarfield->GetMaxLaplacian() - m_scalarfield->GetMinLaplacian()) * hvg / ATFG_V_MAX;
        hvg += m_scalarfield->GetMinLaplacian();
        //m_average_h[i][j] = hvg;
        m_average_hmap->SetValue(hvg, i, j);
        m_max_average_laplacian_2D = fmax(m_max_average_laplacian_2D, hvg);
        m_min_average_laplacian_2D = fmin(m_min_average_laplacian_2D, hvg);
      }
    }

    if (w > 0) {
      g /= w;
      g = m_scalarfield->GetMaxGradient() * g / ATFG_V_MAX;
      m_average_gradient[i] = g;
      m_max_average_gradient = fmax(m_max_average_gradient, g);
      m_min_average_gradient = fmin(m_min_average_gradient, g);

      h /= w;
      h = (m_scalarfield->GetMaxLaplacian() - m_scalarfield->GetMinLaplacian()) * h / ATFG_V_MAX;
      h += m_scalarfield->GetMinLaplacian();
      m_average_laplacian[i] = h;
      m_max_average_laplacian_1D = fmax(m_max_average_laplacian_1D, h);
      m_min_average_laplacian_1D = fmin(m_min_average_laplacian_1D, h);
    }
  }
  return true;
}

void ATFGenerator::SmoothCurveWithGaussian(double* v, const int& n, const int& times)
{
	int t = 0;
	while ( t < times )
	{
		v[0] = (2 * v[0] + v[1]) / 3.0f;
		for ( int i = 1; i < n - 1; ++i )
		{
			v[i] = (v[i - 1] + 2 * v[i] + v[i + 1]) / 4.0f;
		}
		v[n-1] = (2 * v[n-1] + v[n-2]) / 3.0f;
		++t;
	}
}

template<typename T>
T* ATFGenerator::SmoothCurveWithMidpoints(T* v, const int& n, const int& times)
{
	T* midpoints = new T[n];
	memcpy(midpoints, v, n * sizeof(T));

	int t = 0;
	int size = 0;
	while ( t < times )
	{
		size = n - t - 1;
		T* aux = new T[size];
		for ( int i = 0; i < size; ++i )
		{
			aux[i] = (midpoints[i] + midpoints[i + 1]) / 2.0f;
		}

		delete[] midpoints;
		midpoints = aux;

		++t;
	}

	return midpoints;
}

void ATFGenerator::GetValidValuesAndIndexes(double* vin, const int& nin, double*& vout, int*& indexes, int& nout)
{
	int valid_values = 0;
	for ( int i = 0; i < nin; ++i )
		if ( vin[i] != -DBL_MAX && vin[i] != DBL_MAX )
			++valid_values;

	nout = valid_values;
	vout = new double[valid_values];
	indexes = new int[valid_values];
	for ( int i = 0, vi = 0; i < nin; ++i )
	{
		if ( vin[i] != -DBL_MAX && vin[i] != DBL_MAX )
		{
			vout[vi] = vin[i];
			indexes[vi] = i;
			++vi;
		}
	}
}

/// <summary>
/// Gets, for each value, the distance to the closest 
/// boundary associated to it. This information is 
/// extracted from the summed voxel histogram.
/// </summary>
/// <returns>Returns a double array with the distances associated 
/// to all 256 values, ordered by value.</returns>
PredictionMap<double, DoubleCell>* ATFGenerator::GetBoundaryDistancies()
{
	assert(m_scalar_histogram);

  PredictionMap<double, DoubleCell>* distmap = new PredictionMap<double, DoubleCell>(1, ATFG_V_RANGE);
  if (!distmap->Init())
    return NULL;

	double sigma = 2 * m_max_average_gradient / ((m_max_average_laplacian_1D - m_min_average_laplacian_1D) * SQRT_E);
	printf("Sigma: %.2f\n", sigma);

	for ( UINT32 i = 0; i < ATFG_V_RANGE; ++i )
	{
		double g = m_average_gradient[i];
		double l = m_average_laplacian[i];
		if ( g == -DBL_MAX || l == -DBL_MAX )
		{
			continue;
		}
		else
		{
      double val = -sigma * sigma * (l / fmax(g - m_gtresh, 0.000001));
      if (val <= 0.2f && val >= -0.2f)
        distmap->SetValue(val, 0, i);
		}
	}

  return distmap;
}

PredictionMap<double, DoubleCell>* ATFGenerator::GetBoundaryDistancies2D()
{
	assert(m_scalar_histogram);

  PredictionMap<double, DoubleCell>* distmap = new PredictionMap<double, DoubleCell>(ATFG_V_RANGE, ATFG_V_RANGE);
  if (!distmap->Init())
    return NULL;

	double sigma = 2 * m_max_average_gradient / ((m_max_average_laplacian_2D - m_min_average_laplacian_2D) * SQRT_E);
	printf("Sigma: %.2f\n", sigma);

	for (UINT32 i = 0; i < ATFG_V_RANGE; ++i) {
		for (UINT32 j = 0; j < ATFG_V_RANGE; ++j) {
      double g = m_scalarfield->GetMaxGradient() * j / (double)ATFG_V_MAX;
      double l = m_average_hmap->GetValue(i, j);

      if (l != -DBL_MAX)
        distmap->SetValue(-sigma * sigma * (l / fmax(g - m_gtresh, 0.000001)), i, j);
		}
	}
  return distmap;
}

int ATFGenerator::GetMaxPoints(const double* curve, const int* indexes, const int& curve_size, int*& max_indexes)
{
	double* first_derivative = new double[curve_size];
	first_derivative[0] = first_derivative[curve_size - 1] = 0.0f;
	for (int i = 1; i < curve_size - 1; ++i) {
		first_derivative[i] = (curve[i + 1] - curve[i - 1]) / 2.0f;
	}

	double* second_derivative = new double[curve_size];
	second_derivative[0] = second_derivative[curve_size - 1] = 0.0f;
	for (int i = 1; i < curve_size - 1; ++i) {
		second_derivative[i] = (first_derivative[i + 1] - first_derivative[i - 1]) / 2.0f;
	}

	int up = 0, down = 0;
	int* max_indices = new int[curve_size];
	int max_indices_size = 0;
	for (int i = 0; i < curve_size - 1; ++i) {
		if (signbit(first_derivative[i]) ^ signbit(first_derivative[i + 1])) {
			if (second_derivative[i] < 0 && second_derivative[i + 1] < 0)
			{
				if (curve[i] > curve[i + 1])
					max_indices[max_indices_size++] = indexes[i];
				else
					max_indices[max_indices_size++] = indexes[i + 1];
			}
			else if (second_derivative[i] < 0)
			{
				max_indices[max_indices_size++] = indexes[i];
			}
			else if (second_derivative[i + 1] < 0)
			{
				max_indices[max_indices_size++] = indexes[i + 1];
			}
		}
	}

	int* res = new int[max_indices_size];
	memcpy(res, max_indices, max_indices_size*sizeof(int));

	delete[] first_derivative;
	delete[] second_derivative;
	delete[] max_indices;

	max_indexes = res;
	return max_indices_size;
}

int ATFGenerator::GetMinPoints(const double* curve, const int* indexes, const int& curve_size, int*& max_indexes)
{
  double* first_derivative = new double[curve_size];
  first_derivative[0] = first_derivative[curve_size - 1] = 0.0f;
  for (int i = 1; i < curve_size - 1; ++i)
  {
    first_derivative[i] = (curve[i + 1] - curve[i - 1]) / 2.0f;
  }

  double* second_derivative = new double[curve_size];
  second_derivative[0] = second_derivative[curve_size - 1] = 0.0f;
  for (int i = 1; i < curve_size - 1; ++i)
  {
    second_derivative[i] = (first_derivative[i + 1] - first_derivative[i - 1]) / 2.0f;
  }

  int up = 0, down = 0;
  int* min_indices = new int[curve_size];
  int min_indices_size = 0;
  for (int i = 0; i < curve_size - 1; ++i)
  {
    if (!signbit(first_derivative[i]) ^ !signbit(first_derivative[i + 1]))
    {
      if (second_derivative[i] > 0 && second_derivative[i + 1] > 0)
      {
        if (curve[i] < curve[i + 1])
          min_indices[min_indices_size++] = indexes[i];
        else
          min_indices[min_indices_size++] = indexes[i + 1];
      }
      else if (second_derivative[i] > 0)
      {
        min_indices[min_indices_size++] = indexes[i];
      }
      else if (second_derivative[i + 1] > 0)
      {
        min_indices[min_indices_size++] = indexes[i + 1];
      }
    }
  }

  int* res = new int[min_indices_size];
  memcpy(res, min_indices, min_indices_size*sizeof(int));

  delete[] first_derivative;
  delete[] second_derivative;
  delete[] min_indices;

  max_indexes = res;
  return min_indices_size;
}

int ATFGenerator::GetInflectionPoints(const double* curve, const int* indexes, const int& curve_size, int*& inflct_indexes)
{
	double* curve_derivative = new double[curve_size];
	curve_derivative[0] = curve_derivative[curve_size - 1] = 0.0f;
	for (int i = 1; i < curve_size - 1; ++i) {
		curve_derivative[i] = (curve[i + 1] - curve[i - 1]) / 2.0f;
	}
	
	double* first_derivative = new double[curve_size];
	first_derivative[0] = first_derivative[curve_size - 1] = 0.0f;
	for (int i = 1; i < curve_size - 1; ++i) {
		first_derivative[i] = (curve_derivative[i + 1] - curve_derivative[i - 1]) / 2.0f;
	}

	double* second_derivative = new double[curve_size];
	second_derivative[0] = second_derivative[curve_size - 1] = 0.0f;
	for (int i = 1; i < curve_size - 1; ++i) {
		second_derivative[i] = (first_derivative[i + 1] - first_derivative[i - 1]) / 2.0f;
	}

	int up = 0, down = 0;
	int* max_indices = new int[curve_size];
	int max_indices_size = 0;
	for (int i = 0; i < curve_size - 1; ++i) {
		if (signbit(first_derivative[i]) ^ signbit(first_derivative[i + 1])) {
			if (second_derivative[i] > 0 && second_derivative[i + 1] > 0) {
				if (curve[i] > curve[i + 1])
					max_indices[max_indices_size++] = indexes[i];
				else
					max_indices[max_indices_size++] = indexes[i + 1];
			}
			else if (second_derivative[i] > 0) {
				max_indices[max_indices_size++] = indexes[i];
			}
			else if (second_derivative[i + 1] > 0) {
				max_indices[max_indices_size++] = indexes[i + 1];
			}
		}
	}

	int* res = new int[max_indices_size];
	memcpy(res, max_indices, max_indices_size*sizeof(int));

	delete[] curve_derivative;
	delete[] first_derivative;
	delete[] second_derivative;
	delete[] max_indices;

	inflct_indexes = res;
	return max_indices_size;
}