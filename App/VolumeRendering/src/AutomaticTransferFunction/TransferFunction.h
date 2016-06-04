#ifndef TRANSFER_FUNCTION_H
#define TRANSFER_FUNCTION_H

#include "ITransferFunction.h"

#define MAX_V 256

class TransferFunction : public ITransferFunction
{
public:

  /// <summary>
  /// Initializes a new instance of the <see cref="TransferFunction"/> class.
  /// </summary>
  TransferFunction();

  /// <summary>
  /// Generates a transfer function file at a given path.
  /// If a file with the same path already exists, it'll
  /// be replaced.
  /// </summary>
  /// <param name="path">The transfer function file's path.</param>
  bool Generate(const char* path);

  /// <summary>
  /// Sets the color associated to the intensity value.
  /// </summary>
  /// <param name="value">The intensity voxel value.</param>
  /// <param name="red">The red color component.</param>
  /// <param name="green">The green color component.</param>
  /// <param name="blue">The blue color component.</param>
  void SetValueColor(unsigned char value, unsigned char red, unsigned char green, unsigned char blue);

  /// <summary>
  /// Specifies the distance between a intensity value
  /// and its closest boundary. Thus, the input arrays'
  /// size must range from 2 to 256. Any array content
  /// whose index is greater than 255, it'll be ignored.
  /// </summary>
  /// <param name="values">The values array.</param>
  /// <param name="distances">The distances to the closest boundaries.</param>
  /// <param name="sigmas">The sigmas of the boundaries.</param>
  /// <param name="n">The input arrays' size.</param>
  void SetClosestBoundaryDistances(unsigned char* values, float* distances, float* sigmas, int n);

private:
  /// <summary>
  /// The number of color-value associations.
  /// </summary>
  int m_color_size;
  /// <summary>
  /// The number of intensity values whose distance to
  /// closest boundary was given.
  /// </summary>
  int m_values_size;
  /// <summary>
  /// It storages if a color was setted by SetValueColor
  /// for each intensity value.
  /// </summary>
  bool m_has_color[MAX_V];
  /// <summary>
  /// It storages the color values setted by SetValueColor.
  /// </summary>
  unsigned char m_color[3 * MAX_V];
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
  /// It storages the sigmas values setted by
  /// SetClosestBoundaryDistances.
  /// </summary>
  float* m_sigma;
};

#endif