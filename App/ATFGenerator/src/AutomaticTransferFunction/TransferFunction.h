/// TransferFunction.h
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#ifndef TRANSFER_FUNCTION_H
#define TRANSFER_FUNCTION_H

#include "ITransferFunction.h"
#include <iup.h>

/// <summary>
/// This class gives an implementation to the ITransferFunction
/// interface. It allows to generate transfer function
/// files with a "tf1d" extension.
/// </summary>
/// <seealso cref="ITransferFunction" />
class TransferFunction : public ITransferFunction
{
public:

  /// <summary>
  /// Initializes a new instance of the <see cref="TransferFunction" /> class.
  /// </summary>
  /// <param name="path">The full path to the transfer
  /// function file. Extension isn't needed and it'll be
  /// considered part of the file's name, if passed.</param>
  TransferFunction(const char* path);

  /// <summary>
  /// Finalizes an instance of the <see cref="TransferFunction"/> class.
  /// </summary>
  ~TransferFunction();

  /// <summary>
  /// Generates a transfer function file at a given path.
  /// If a file with the same path already exists, it'll
  /// be replaced.
  /// </summary>
  /// <returns>Returns true if the transfer function can
  /// be generated, false otherwise.</returns>
  bool Generate();

  void SetBoundaryThickness(int thickness)
  {
    m_thickness = thickness;
  }

  void SetBoundary(int boundary)
  {
    m_boundary = boundary;
  }

  void SetTransferFunctionPlot(Ihandle * ih)
  {
    m_tf_plot = ih;
  }

  void SetBoundaryFunctionPlot(Ihandle * ih)
  {
    m_bx_plot = ih;
  }

  /// <summary>
  /// Specifies the distance between a intensity value
  /// and its closest boundary. Thus, the input arrays'
  /// size must range from 2 to 256. Any array content
  /// whose index is greater than 255, it'll be ignored.
  /// </summary>
  /// <param name="values">The values array.</param>
  /// <param name="distances">The distances to the closest boundaries.</param>
  /// <param name="n">The input arrays' size.</param>
  void SetClosestBoundaryDistances(unsigned char* values, float* distances, int n);

private:
  float CenteredTriangleFunction(float max, float base, float x);

private:
  /// <summary>
  /// It storages the values setted by
  /// SetClosestBoundaryDistances.
  /// </summary>
  unsigned char* m_value;
  /// <summary>
  /// It storages the distances setted by 
  /// SetClosestBoundaryDistances.
  /// </summary>
  float* m_distance;
  /// <summary>
  /// The boundary's sigma
  /// </summary>
  int m_thickness;
  int m_boundary;
  Ihandle * m_tf_plot;
  Ihandle * m_bx_plot;
};

#endif