/// ITransferFunction.h
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#ifndef I_TRANSFER_FUNCTION_H
#define I_TRANSFER_FUNCTION_H

#include <stdexcept>
#include <string.h>

#define MAX_V 256
#define TF_EXT ".tf1d"

/// <summary>
/// This interface encapsulates the Transfer Function
/// baic concept to this project, which is to be able
/// to generate a transfer funciton file in a specific
/// manner.
/// </summary>
class ITransferFunction
{
public:
  /// <summary>
  /// Initializes a new instance of the <see cref="ITransferFunction" /> class.
  /// </summary>
  /// <param name="path">The full path to the transfer
  /// function file.</param>
  /// <param name="ext">The file extension.</param>
  ITransferFunction()
  {
  }

  ~ITransferFunction()
  {
  }

  /// <summary>
  /// The implementation of this function must generate a
  /// transfer function at the given path.
  /// A transfer function file has the extension "tf1d" and
  /// its structure is as follows:
  /// - First line: Always considered a comment.
  /// - Second line: Number 'nc' of color attributes.
  /// - Next 'nc' lines: R G B V. Where R, G and B are the
  /// rgb float components associated to a value V.
  /// - Next line: Number 'no' of opacity attribute.
  /// - Next 'no' lines: O V. Where O is an opacity
  /// associated to a value V.
  /// </summary>
  virtual bool Generate() = 0;

  /// <summary>
  /// Sets the color associated to the intensity value.
  /// </summary>
  /// <param name="value">The intensity voxel value.</param>
  /// <param name="red">The red color component.</param>
  /// <param name="green">The green color component.</param>
  /// <param name="blue">The blue color component.</param>
  void SetValueColor(unsigned char value, unsigned char red, unsigned char green, unsigned char blue)
  {
    m_color_size++;
    m_has_color[value] = true;
    m_color[(value * 3)] = red;
    m_color[(value * 3) + 1] = green;
    m_color[(value * 3) + 2] = blue;
  }

  /// <summary>Gets the path.</summary>
  /// <returns>Returns the complete file path to the
  /// transfer function file.</returns>
  char* GetPath()
  {
    return m_path;
  }

protected:
  /// <summary>
  /// The transfer function file path.
  /// </summary>
  char* m_path;

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
};

#endif