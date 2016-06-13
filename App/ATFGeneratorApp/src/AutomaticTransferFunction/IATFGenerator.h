/// IATFGenerator.h
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#ifndef IATFGenerator_H
#define IATFGenerator_H

#include <volrend\volume.h>

class ITransferFunction;

/// <summary>
/// This interface encapsulates the Transfer Function
/// Generator baic concept, which is to construct a
/// transfer funciton given a volume as input.
/// </summary>
class IATFGenerator
{
public:
  /// <summary>
  /// Initializes a new instance of the <see cref="IATFGenerator"/> class.
  /// </summary>
  /// <param name="volume">The volume whose tranfer function will be constructed.</param>
  IATFGenerator(vr::Volume* volume)
  {
    if (!volume)
      throw std::exception_ptr();

    m_volume = volume;
    m_width = volume->GetWidth();
    m_height = volume->GetHeight();
    m_depth = volume->GetDepth();
  }

  /// <summary>
  /// Gets an instance of the Transfer Function generated
  /// automatically.
  /// </summary>
  /// <returns>Returns a Transfer Function.</returns>
  virtual ITransferFunction* GetTransferFunction() = 0;

protected:
  /// <summary>
  /// The width of the volume.
  /// </summary>
  unsigned int m_width;
  /// <summary>
  /// The height of the volume.
  /// </summary>
  unsigned int m_height;
  /// <summary>
  /// The depth of the volume.
  /// </summary>
  unsigned int m_depth;
  /// <summary>
  /// A reference to the volume.
  /// </summary>
  vr::Volume* m_volume;
};

#endif