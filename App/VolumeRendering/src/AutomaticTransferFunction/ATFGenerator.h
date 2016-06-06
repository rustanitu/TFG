/// ATFGenerator.h
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#ifndef ATFGenerator_H
#define ATFGenerator_H

#include "IATFGenerator.h"
#include <volrend/Volume.h>

/*************************/
/* Constants Definitions */
/*************************/

#define SQRT_E 1.6487212707
#define MAX_V 256

class TransferFunction;

/// <summary>
/// This classes implements an Automatic Transfer Function
/// Generator, based on Kindlmann's paper "Semi-automatic
/// Generation of Transfer Functions for Direct Volume
/// Rendering" (http://doi.acm.org/10.1145/288126.288167).
/// Given a volume with greatest intensity value equals to
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
  /// <param name="volume">The volume whose transfer 
  /// function one wants to extract.</param>
  ATFGenerator(vr::Volume* volume);
  /// <summary>
  /// Finalizes an instance of the 
  /// <see cref="ATFGenerator"/> class.
  /// </summary>
  ~ATFGenerator();

public:
  /// <summary>Prepare this instance creating the
  /// necessary memory.</summary>
  /// <returns>Returns true if all the memory needed 
  /// was successfully created. Otherwise, it returns false.</returns>
  bool Init();
  /// <summary>
  /// Extract a transfer function from the volume, based in 
  /// Kindlmann's paper. This function needs to be called
  /// only once, to do all the calculations.
  /// </summary>
  /// <returns>Returns true if the transfer function can be
  /// generated. False, otherwise.</returns>
  bool ExtractTransferFunction();
  /// <summary>
  /// Gets an aproximation of the voxel's gradient, using 
  /// its first derivatives.
  /// </summary>
  /// <param name="x">The voxel's x component.</param>
  /// <param name="y">The voxel's y component.</param>
  /// <param name="z">The voxel's z component.</param>
  /// <returns>Returns the float aproximated gradient.</returns>
  float GetGradient(int x, int y, int z);
  /// <summary>
  /// Gets an aproximation of the voxel's laplacian, using 
  /// its second derivatives.
  /// </summary>
  /// <param name="x">The voxel's x component.</param>
  /// <param name="y">The voxel's y component.</param>
  /// <param name="z">The voxel's z component.</param>
  /// <returns>Returns the float aproximated laplacian.</returns>
  float GetLaplacian(int x, int y, int z);
  /// <summary>
  /// Generates a PGM image file with a histogram slice.
  /// The image is generated in the working directory and 
  /// it's named "Histogram Slice v", where 'v' is the 
  /// value input.
  /// </summary>
  /// <param name="v">The value whose slice it's desired.</param>
  void GenerateHistogramSlice(int v);
  /// <summary>
  /// Generates the all the 256 histogram slices, trhought
  /// calls to void GenerateHistogramSlice(int v).
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
  /// <summary>
  /// Gets the transfer function.
  /// </summary>
  /// <returns>Returns a pointer to the transfer function generated automatically.</returns>
  ITransferFunction* GetTransferFunction();

private:
  /// <summary>
  /// Calculates an aproximation of the voxel's gradient, 
  /// using its first derivatives.
  /// </summary>
  /// <param name="x">The voxel's x component.</param>
  /// <param name="y">The voxel's y component.</param>
  /// <param name="z">The voxel's z component.</param>
  /// <returns>Returns the float aproximated gradient.</returns>
  float CalculateGradient(int x, int y, int z);
  /// <summary>
  /// Calculates an aproximation of the voxel's laplacian, 
  /// using its second derivatives.
  /// </summary>
  /// <param name="x">The voxel's x component.</param>
  /// <param name="y">The voxel's y component.</param>
  /// <param name="z">The voxel's z component.</param>
  /// <returns>Returns the float aproximated laplacian.</returns>
  float CalculateLaplacian(int x, int y, int z);
  /// <summary>
  /// Iterates over the volume, calculating the gradient 
  /// and the laplacian values for each voxel.
  /// </summary>
  /// <returns>Returns true if all the memory needed 
  /// was successfully created. Otherwise, it returns false.</returns>
  bool CalculateVolumeDerivatives();
  /// <summary>
  /// Generates a 3D histogram which accumulates 
  /// occurrences of value-gradient-laplacian triplets.
  /// The volume derivatives must have been calculated 
  /// before this call.
  /// </summary>
  /// <returns>Returns true if the histogram could be generated. 
  /// False, otherwise.</returns>
  bool GenerateHistogram();
  /// <summary>
  /// Gets, for each value, the distance to the closest 
  /// boundary associated to it. This information is 
  /// extracted from the summed voxel histogram.
  /// </summary>
  /// <returns>Returns a float array with the distances associated 
  /// to all 256 values, ordered by value.</returns>
  float* GetBoundaryDistancies();
  /// <summary>
  /// Gets the sigma value of the closest boundary associated the specified value.
  /// </summary>
  /// <param name="v">The intendity value.</param>
  /// <returns>Returns the float sigma value.</returns>
  float GetSigma(unsigned char v);

private:
  /// <summary>
  /// An array that storages the maximum gradient value for every intensity value.
  /// </summary>
  long m_max_gradient[256];
  /// <summary>
  /// An array that storages the maximum laplacian value for every intensity value.
  /// </summary>
  long m_max_laplacian[256];
  /// <summary>
  /// An array that storages the minimum laplacian value for every intensity value.
  /// </summary>
  long m_min_laplacian[256];
  /// <summary>
  /// The global maximum gradient value.
  /// </summary>
  long m_max_global_gradient;
  /// <summary>
  /// The global maximum laplacian value.
  /// </summary>
  long m_max_global_laplacian;
  /// <summary>
  /// The global minimum laplacian value.
  /// </summary>
  long m_min_global_laplacian;
  /// <summary>
  /// The gradient of each volume voxel.
  /// </summary>
  float* m_scalar_gradient;
  /// <summary>
  /// The laplacian of each volume voxel.
  /// </summary>
  float* m_scalar_laplacian;
  /// <summary>
  /// The average gradient of each intensity value.
  /// </summary>
  float m_average_gradient[256];
  /// <summary>
  /// The average laplacian of each intensity value.
  /// </summary>
  float** m_average_laplacian;
  /// <summary>
  /// Histogram of occurences for each triple value x gradient x laplacian.
  /// </summary>
  unsigned char*** m_scalar_histogram;
  /// <summary>
  /// The transfer function generated automatically.
  /// </summary>
  TransferFunction* m_transfer_function;
};

#endif