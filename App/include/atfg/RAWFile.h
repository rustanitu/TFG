/// RAWFile.h
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#ifndef RAW_FILE_H
#define RAW_FILE_H

#include <fstream>

#define RAW_EXT ".raw"

/// <summary>
/// This class implements an volume file writer in a raw
/// format. The file must be openned before any write
/// instruction and closed when there is nothing else
/// to write in it.
/// </summary>
class RAWFile
{
public:
  /// <summary>
  /// Initializes a new instance of the <see cref="RAWFile"/> class.
  /// </summary>
  /// <param name="path">The full path to the file.
  /// Extension isn't needed and it'll be considered part
  /// of the file's name, if passed</param>
  /// <param name="width">The volume width.</param>
  /// <param name="height">The volume height.</param>
  RAWFile(const char* path, unsigned int width, unsigned int height, unsigned int depth);
  
  /// <summary>
  /// Finalizes an instance of the <see cref="RAWFile"/> class.
  /// </summary>
  ~RAWFile();

  /// <summary>
  /// Opens the volume file.
  /// </summary>
  /// <returns>Returns true if the file was successfully openned.
  /// Otherwise it returns false.</returns>
  bool Open();

  /// <summary>
  /// Writes a byte to the volume. The file must be openned
  /// before.
  /// </summary>
  /// <param name="byte">The byte.</param>
  void WriteByte(unsigned char byte);

  /// <summary>
  /// Writes a byte sequence to the volume. The file must be
  /// openned before.
  /// </summary>
  /// <param name="byte">The byte.</param>
  void WriteByte(const char* byte);

  /// <summary>
  /// Writes a end line to the volume. The file must be openned
  /// before.
  /// </summary>
  void WriteEndLine();

  /// <summary>
  /// Closes the volume file.
  /// </summary>
  void Close();

private:
  /// <summary>
  /// The file path.
  /// </summary>
  char* m_path;

  /// <summary>
  /// The volume width.
  /// </summary>
  unsigned int m_width;

  /// <summary>
  /// The volume height.
  /// </summary>
  unsigned int m_height;

  /// <summary>
  /// The volume depth.
  /// </summary>
  unsigned int m_depth;

  /// <summary>
  /// Reference to the file.
  /// </summary>
  FILE* m_file;
};

#endif