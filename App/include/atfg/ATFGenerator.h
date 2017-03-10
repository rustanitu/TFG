/// ATFGenerator.h
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#ifndef ATFGenerator_H
#define ATFGenerator_H

#include "IATFGenerator.h"
#include "TransferFunction1D.h"
#include "TransferFunction2D.h"
#include <iup.h>
#include "PredictionMap.h"

/*************************/
/* Constants Definitions */
/*************************/

#define SQRT_E 1.6487212707
#define ATFG_V_RANGE 256
#define ATFG_V_MAX (ATFG_V_RANGE - 1)

class TransferFunction;

/// <summary>
/// This classes implements an Automatic Transfer Function
/// Generator, based on Kindlmann's paper "Semi-automatic
/// Generation of Transfer Functions for Direct ScalarField
/// Rendering" (http://doi.acm.org/10.1145/288126.288167).
/// Given a scalarfield with greatest intensity value equals to
/// 255, this class provides functions to generate a good
/// transfer function automatically.
/// </summary>
/// <seealso cref="IATFGenerator" />
class ATFGenerator : public IATFGenerator
{
public:
	/// <summary>
	/// Initializes a new instance of the 
	/// <see cref="ATFGenerator" /> class.
	/// </summary>
	/// <param name="scalarfield">The scalarfield whose transfer 
	/// function one wants to extract.</param>
	ATFGenerator (vr::ScalarField* scalarfield);

	/// <summary>
	/// Finalizes an instance of the 
	/// <see cref="ATFGenerator"/> class.
	/// </summary>
	~ATFGenerator();

public:
	/// <summary>It does all the math necessary so information
	/// can be extracted from it. The user will not be able to
	/// get any kind of information without a successfull call
	/// to this function. </summary>
	/// <returns>Returns true if the instance was correctly
	/// initialized and false otherwise.</returns>
	bool Init();

	/// <summary>
	/// Extract a transfer function from the scalarfield, based in 
	/// Kindlmann's paper. This function needs to be called
	/// only once, to do all the calculations.
	/// </summary>
	/// <returns>Returns true if the transfer function can be
	/// generated. False, otherwise.</returns>
	bool ExtractTransferFunction();

	/// <summary>
	/// Gets the voxel's value.
	/// </summary>
	/// <param name="x">The voxel's x component.</param>
	/// <param name="y">The voxel's y component.</param>
	/// <param name="z">The voxel's z component.</param>
	/// <returns>Returns the double aproximated gradient.</returns>
	double GetValue(int x, int y, int z);

	/// <summary>
	/// Gets an aproximation of the voxel's gradient, using 
	/// its first derivatives.
	/// </summary>
	/// <param name="x">The voxel's x component.</param>
	/// <param name="y">The voxel's y component.</param>
	/// <param name="z">The voxel's z component.</param>
	/// <returns>Returns the double aproximated gradient.</returns>
	double GetGradient(const UINT32& x, const UINT32& y, const UINT32& z);

	/// <summary>
	/// Gets an aproximation of the voxel's laplacian, using 
	/// its second derivatives.
	/// </summary>
	/// <param name="x">The voxel's x component.</param>
	/// <param name="y">The voxel's y component.</param>
	/// <param name="z">The voxel's z component.</param>
	/// <returns>Returns the double aproximated laplacian.</returns>
	double GetLaplacian(const UINT32& x, const UINT32& y, const UINT32& z);

	/// <summary>
	/// Generates a PGM image file with a scalarfield slice.
	/// The image is generated in the working directory and 
	/// it's named "ScalarField Slice v", where 'v' is the 
	/// value input.
	/// </summary>
	/// <param name="v">The value whose slice it's desired.</param>
	void GenerateVolumeSlice(const UINT32& v);
	void GenerateGradientSlice(const UINT32& v);
	void GenerateLaplacianSlice(const UINT32& v);

	/// <summary>
	/// Generates the all the scalarfield slices, trhought
	/// calls to void GenerateVolumeSlice(const UINT32& v).
	/// </summary>
	void GenerateVolumeSlices();
	void GenerateGradientSlices();
	void GenerateLaplacianSlices();

	/// <summary>
	/// Generates a PGM image file with a histogram slice.
	/// The image is generated in the working directory and 
	/// it's named "Histogram Slice v", where 'v' is the 
	/// value input.
	/// </summary>
	/// <param name="v">The value whose slice it's desired.
	/// The value must range from 0 to 255</param>
	bool GenerateHistogramSlice(const UINT32& v);

	/// <summary>
	/// Generates the all the 256 histogram slices, trhought
	/// calls to void GenerateHistogramSlice(const UINT32& v).
	/// </summary>
	void GenerateHistogramSlices();

	/// <summary>
	/// Acumulates the gradient values of the 3D histogram, 
	/// generating a 2D hitogram of laplacian values per 
	/// intensity values. This 2D histogram is generated
	/// as an PGM image file in the working directory, 
	/// named "Gradient Summed Histogram".
	/// </summary>
	void GenerateGradientSummedHistogram();

	/// <summary>
	/// Acumulates the laplacian values of the 3D histogram, 
	/// generating a 2D hitogram of gradient values per 
	/// intensity values. This 2D histogram is generated
	/// as an PGM image file in the working directory, 
	/// named "Laplacian Summed Histogram".
	/// </summary>
	void GenerateLaplacianSummedHistogram();

	void GenerateDataChart();

	/// <summary>
	/// Gets the transfer function.
	/// </summary>
	/// <returns>Returns a pointer to the transfer function generated automatically.</returns>
	vr::TransferFunction* GetTransferFunction();

	void SetGTresh(double gt)
	{
		m_gtresh = gt * GetMaxAverageGradient() * 0.01f;
	}

  double GetGTresh()
  {
    return m_gtresh;
  }

	void SetMinimumHistogramValue(UINT32 min)
	{
		m_min_hist = min;
		GenerateHistogram();
	}

	void SetDerivativePlot(Ihandle * ih)
	{
		m_deriv_plot = ih;
	}

	void SetBoundaryDistancePlot(Ihandle * ih)
	{
		m_dist_plot = ih;
	}

	double GetMinAverageGradient()
	{
		return m_min_average_gradient;
	}

	double GetMaxAverageGradient()
	{
		return m_max_average_gradient;
	}

  void SetTF1D()
  {
    m_tf1d = true;
    m_tfmode_changed = true;
  }

  void SetTF2D()
  {
    m_tf1d = false;
    m_tfmode_changed = true;
  }

  bool UpdateVolumeDerivatives();

private:
	/// <summary>
	/// Iterates over the scalarfield, calculating the gradient 
	/// and the laplacian values for each voxel.
	/// </summary>
	/// <returns>Returns true if all the memory needed 
	/// was successfully created. Otherwise, it returns false.</returns>
	bool CalculateVolumeDerivatives();

	/// <summary>
	/// Generates a 3D histogram which accumulates 
	/// occurrences of value-gradient-laplacian triplets.
	/// The scalarfield derivatives must have been calculated 
	/// before this call.
	/// </summary>
	/// <returns>Returns true if the histogram could be generated. 
	/// False, otherwise.</returns>
	bool GenerateHistogram();

  bool EstimateAverageValues();

  PredictionMap<double, DoubleCell>* GetBoundaryDistancies();
  PredictionMap<double, DoubleCell>* GetBoundaryDistancies2D();

	void SmoothCurves();

	void SmoothCurveWithGaussian(double* v, const int& n, const int& times);

	template<typename T>
	T* SmoothCurveWithMidpoints(T* v, const int& n, const int& times);

	void GetValidValuesAndIndexes(double* vin, const int& nin, double*& vout, int*& indexes, int& nout);

	void SetDefaultColor();

	int GetMaxPoints(const double* curve, const int* indexes, const int& curve_size, int*& max_indexes);
  int GetMinPoints(const double* curve, const int* indexes, const int& curve_size, int*& max_indexes);
	int GetInflectionPoints(const double* curve, const int* indexes, const int& curve_size, int*& inflct_indexes);

private:
  bool m_tf1d;
  bool m_tfmode_changed;

  double m_max_average_gradient;
	double m_min_average_gradient;
	double m_max_average_laplacian_1D;
	double m_min_average_laplacian_1D;
  double m_max_average_laplacian_2D;
  double m_min_average_laplacian_2D;

	/// <summary>
	/// The gradient of each scalarfield voxel.
	/// </summary>
	double* m_scalar_gradient;

	/// <summary>
	/// The laplacian of each scalarfield voxel.
	/// </summary>
	double* m_scalar_laplacian;

	double m_average_gradient[ATFG_V_RANGE];
	double m_average_laplacian[ATFG_V_RANGE];
	//double m_average_h[ATFG_V_RANGE][ATFG_V_RANGE];
  PredictionMap<double, DoubleCell>* m_average_hmap;

	int m_max_size;
	int* m_max_indexes;
	int m_inflct_size;
	int* m_inflct_indexes;

	/// <summary>
	/// Histogram of occurences for each triple value x gradient x laplacian.
	/// </summary>
	unsigned char m_scalar_histogram[ATFG_V_RANGE][ATFG_V_RANGE][ATFG_V_RANGE];

	/// <summary>
	/// The transfer function generated automatically.
	/// </summary>
	vr::TransferFunction* m_transfer_function;

	/// <summary>
	/// Inidicates if the instance has already been
	/// initialized.
	/// </summary>
	bool m_initialized;

	double m_gtresh;
	UINT32 m_min_hist;

	Ihandle* m_deriv_plot;
	Ihandle* m_dist_plot;
};

#endif