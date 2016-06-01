#ifndef TRANSFER_FUNCTION_H
#define TRANSFER_FUNCTION_H

#include "ITransferFunction.h"

#define MAX_V 256

class TransferFunction : public ITransferFunction
{
public:
  /// <summary>
  /// Generates a transfer function file at a given path.
  /// If a file with the same path already exists, it'll
  /// be replaced.
  /// </summary>
  /// <param name="path">The transfer function file's path.</param>
  void Generate(const char* path);

  void SetValueColor(unsigned char value, unsigned char red, unsigned char green, unsigned char blue);

  void SetValueOpacity(unsigned char value, unsigned char opacity);

  void SetClosestBoundaryDistances(float* distances);

private:
  unsigned char m_color[4 * MAX_V];
  unsigned char m_opacity[2 * MAX_V];
  float m_distance[MAX_V];
};

#endif