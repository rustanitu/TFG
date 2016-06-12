/// ITransferFunction.h
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#ifndef I_TRANSFER_FUNCTION_H
#define I_TRANSFER_FUNCTION_H

#include <string.h>

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
  ITransferFunction(const char* path, const char* ext)
  {
    int path_len = strlen(path);
    int ext_len = strlen(ext);
    int length = path_len + ext_len;
    m_path = new char[length + 1];
    m_path = (char*)memcpy(m_path, path, path_len);
    memcpy(m_path + path_len, ext, ext_len);
    m_path[length] = '\0';
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
};

#endif