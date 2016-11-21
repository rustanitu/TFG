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


#define ATFG_GAMA_CORRECTION 0.33f

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
, m_min_hist(1)
, m_deriv_plot(NULL)
, m_tf_plot(NULL)
, m_dist_plot(NULL)
, m_max_average_gradient(-FLT_MAX)
, m_min_average_gradient(FLT_MAX)
, m_max_average_laplacian(-FLT_MAX)
, m_min_average_laplacian(FLT_MAX)
{
	printf("ATFGenerator criado.\n");
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

	m_initialized = true;
	printf("ATFGenerator Inicializado.\n");
	printf("--------------------------------------------------\n");
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
	if ( !m_initialized )
		throw std::domain_error("Instance not initialized. Init must be called once!\n");

	delete m_transfer_function;
	m_transfer_function = new vr::TransferFunction1D();
	//m_transfer_function->SetVolume(m_scalarfield);
	m_transfer_function->SetName(std::string("AutomaticTransferFunction"));
	m_transfer_function->SetTransferFunctionPlot(m_tf_plot);
	m_transfer_function->SetBoundaryFunctionPlot(m_dist_plot);
	SetDefaultColor();

	float* x = new float[ATFG_V_RANGE];
	unsigned char* v = new unsigned char[ATFG_V_RANGE];
	if ( !x || !v )
	{
		printf("Erro - Nao ha memoria suficiente para extrair a funcao de transferencia!\n");
		return false;
	}

	UINT32 n_v;
	float sigma = GetBoundaryDistancies(x, v, &n_v);
	m_transfer_function->SetClosestBoundaryDistances(v, x, n_v);
	GenerateDataValuesFile(x, v, n_v);
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
	return m_scalarfield->GetValue(x, y, z);
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
/// <returns>Returns the float aproximated laplacian.</returns>
float ATFGenerator::GetLaplacian(const UINT32& x, const UINT32& y, const UINT32& z)
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
			float g = m_scalarfield->GetScalarGradient(m_scalar_gradient[m_scalarfield->GetId(i, j - 1, k)], ATFG_V_MAX);
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
			float l = m_scalarfield->GetScalarLaplacian(m_scalar_laplacian[m_scalarfield->GetId(i, j - 1, k)], ATFG_V_MAX);
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
			unsigned char val = (unsigned char) (ATFG_V_MAX * pow(float(histogram[i][j - 1]) / ATFG_V_MAX, ATFG_GAMA_CORRECTION));
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
			unsigned char val = (unsigned char) (ATFG_V_MAX * pow(float(histogram[i][j - 1]) / ATFG_V_MAX, ATFG_GAMA_CORRECTION));
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
			unsigned char val = (unsigned char) (ATFG_V_MAX * pow(float(histogram[i][j - 1]) / ATFG_V_MAX, ATFG_GAMA_CORRECTION));
			pgmfile.WriteByte(val);
		}
		pgmfile.WriteEndLine();
	}
	pgmfile.Close();
}

void ATFGenerator::GenerateDataValuesFile(float *x, unsigned char *v, const UINT32& n)
{
	if ( !m_initialized )
		throw std::domain_error("Instance not initialized. Init must be called once!\n");

	IupSetAttribute(m_deriv_plot, "CLEAR", "YES");

	IupPlotBegin(m_deriv_plot, 0);
	for ( UINT32 i = 0; i < ATFG_V_RANGE; i++ )
		if ( m_average_gradient[i] != -FLT_MAX )
			IupPlotAdd(m_deriv_plot, m_scalarfield->ConvertScalarToValue(i, ATFG_V_MAX), m_average_gradient[i]);
	IupPlotEnd(m_deriv_plot);
	IupSetAttribute(m_deriv_plot, "DS_MODE", "MARKLINE");
	IupSetAttribute(m_deriv_plot, "DS_MARKSTYLE", "PLUS");
	IupSetAttribute(m_deriv_plot, "DS_MARKSIZE", "5");
	IupSetAttribute(m_deriv_plot, "DS_NAME", "g(v)");
	IupSetAttribute(m_deriv_plot, "DS_COLOR", "0 0 128");

	IupPlotBegin(m_deriv_plot, 0);
	for ( UINT32 i = 0; i < ATFG_V_RANGE; i++ )
		if ( m_average_laplacian[i] != -FLT_MAX )
			IupPlotAdd(m_deriv_plot, m_scalarfield->ConvertScalarToValue(i, ATFG_V_MAX), m_average_laplacian[i]);
	IupPlotEnd(m_deriv_plot);
	IupSetAttribute(m_deriv_plot, "DS_MODE", "MARKLINE");
	IupSetAttribute(m_deriv_plot, "DS_MARKSTYLE", "PLUS");
	IupSetAttribute(m_deriv_plot, "DS_MARKSIZE", "5");
	IupSetAttribute(m_deriv_plot, "DS_NAME", "h(v)");
	IupSetAttribute(m_deriv_plot, "DS_COLOR", "0 128 0");

	IupPlotBegin(m_deriv_plot, 0);
	IupPlotAdd(m_deriv_plot, 0, 0);
	IupPlotAdd(m_deriv_plot, m_scalarfield->GetMaxValue(), 0);
	IupPlotEnd(m_deriv_plot);
	IupSetAttribute(m_deriv_plot, "DS_NAME", "0");
	IupSetAttribute(m_deriv_plot, "DS_COLOR", "0 0 0");

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

	m_scalar_gradient = new float[size];
	m_scalar_laplacian = new float[size];

	if ( !m_scalar_gradient || !m_scalar_laplacian )
	{
		printf("Erro - Nao ha memoria suficiente para processar o scalarfield!\n");
		return false;
	}

	for ( UINT32 x = 0; x < m_width; ++x )
	{
		for ( UINT32 y = 0; y < m_height; ++y )
		{
			for ( UINT32 z = 0; z < m_depth; ++z )
			{
				UINT32 id = m_scalarfield->GetId(x, y, z);
				//if ( x * y * z == 0 || x == m_width - 1 || y == m_height - 1 || z == m_depth - 1 )
				//	m_scalar_gradient[id] = 0.0f;
				//else
					m_scalar_gradient[id] = m_scalarfield->CalculateGradient(x, y, z);
			}
		}
	}

	for ( UINT32 x = 0; x < m_width; ++x )
	{
		for ( UINT32 y = 0; y < m_height; ++y )
		{
			for ( UINT32 z = 0; z < m_depth; ++z )
			{
				UINT32 id = m_scalarfield->GetId(x, y, z);
				//if (x * y * z == 0 || x == m_width - 1 || y == m_height - 1 || z == m_depth - 1)
				//	m_scalar_laplacian[id] = -FLT_MAX;
				//else
					m_scalar_laplacian[id] = m_scalarfield->CalculateLaplacian(x, y, z);
			}
		}
	}

	for ( UINT32 x = 0; x < m_width; ++x )
	{
		for ( UINT32 y = 0; y < m_height; ++y )
		{
			for ( UINT32 z = 0; z < m_depth; ++z )
			{
				UINT32 id = m_scalarfield->GetId(x, y, z);
				if ( m_scalar_laplacian[id] == -FLT_MAX )
					m_scalar_laplacian[id] = m_scalarfield->GetMinLaplacian();
			}
		}
	}

	printf("MaxGradient: %.2f\n", m_scalarfield->GetMaxGradient());
	printf("MinLaplacian: %.2f\n", m_scalarfield->GetMinLaplacian());
	printf("MaxLaplacian: %.2f\n", m_scalarfield->GetMaxLaplacian());

	printf("Derivadas calculadas.\n");
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

	int histgrad_accum[ATFG_V_RANGE][ATFG_V_RANGE];
	int histlapl_accum[ATFG_V_RANGE][ATFG_V_RANGE];

	for ( UINT32 i = 0; i < ATFG_V_RANGE; ++i )
	{
		for ( UINT32 j = 0; j < ATFG_V_RANGE; ++j )
		{
			histgrad_accum[i][j] = 0;
			histlapl_accum[i][j] = 0;
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
				unsigned char l = m_scalarfield->GetScalarLaplacian(m_scalar_laplacian[vol_id], ATFG_V_MAX);

				if ( m_scalar_histogram[v][g][l] < ATFG_V_MAX )
					m_scalar_histogram[v][g][l]++;
			}
		}
	}

	printf("Extraindo derivadas medias, pelo histograma.\n");

	for ( UINT32 i = 0; i < ATFG_V_RANGE; ++i )
	{
		for ( UINT32 j = 0; j < ATFG_V_RANGE; ++j )
		{
			for ( UINT32 k = 0; k < ATFG_V_RANGE; ++k )
			{
				histgrad_accum[i][j] += m_scalar_histogram[i][j][k];
				histlapl_accum[i][k] += m_scalar_histogram[i][j][k];
			}
		}
	}

	m_max_average_gradient = -FLT_MAX;
	m_min_average_gradient = FLT_MAX;
	m_max_average_laplacian = -FLT_MAX;
	m_min_average_laplacian = FLT_MAX;

	// Calculate average laplacian and gradient
	for ( UINT32 i = 0; i < ATFG_V_RANGE; ++i )
	{
		m_average_gradient[i] = -FLT_MAX;
		m_average_laplacian[i] = -FLT_MAX;
		UINT32 w = 0;
		float g = 0.0f;
		float h = 0.0f;
		for ( UINT32 j = 0; j < ATFG_V_RANGE; ++j )
		{
			for ( UINT32 k = 0; k < ATFG_V_RANGE; ++k )
			{
				if ( m_scalar_histogram[i][j][k] > 0)// && (histgrad_accum[i][j] > m_min_hist && histlapl_accum[i][k] > m_min_hist ))
				{
					g += j;
					h += k;
					w++;
				}
			}
		}

		if ( w > 0 )
		{
			g /= w;
			g = m_scalarfield->GetMaxGradient() * g / ATFG_V_MAX;
			m_average_gradient[i] = g;
			m_max_average_gradient = fmax(m_max_average_gradient, g);
			m_min_average_gradient = fmin(m_min_average_gradient, g);

			h /= w;
			h = (m_scalarfield->GetMaxLaplacian() - m_scalarfield->GetMinLaplacian()) * h / ATFG_V_MAX;
			h += m_scalarfield->GetMinLaplacian();
			m_average_laplacian[i] = h;
			m_max_average_laplacian = fmax(m_max_average_laplacian, h);
			m_min_average_laplacian = fmin(m_min_average_laplacian, h);
		}

		printf("g(%d): %.2f,\th(%d): %.2f\n", i, m_average_gradient[i], i, m_average_laplacian[i]);
	}

	printf("G(v): %.2f\t-\t%.2f\n", m_min_average_gradient, m_max_average_gradient);
	printf("H(v): %.2f\t-\t%.2f\n", m_min_average_laplacian, m_max_average_laplacian);

	printf("Histograma gerado.\n");
	printf("--------------------------------------------------\n");

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

	if (m_max_average_laplacian == 0.0f)
	{
		x = NULL;
		v = NULL;
		*n = 0;
		return 0.0f;
	}

	//float sigma = 2 * SQRT_E * m_max_average_gradient / (m_max_average_laplacian - m_min_average_laplacian);
	float sigma = m_max_average_gradient / (m_max_average_laplacian * SQRT_E);
	printf("Sigma: %.2f\n", sigma);

	UINT32 c = 0;
	for ( UINT32 i = 0; i < ATFG_V_RANGE; ++i )
	{
		float g = m_average_gradient[i];
		float l = m_average_laplacian[i];
		if ( g == -FLT_MAX || l == -FLT_MAX )
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
