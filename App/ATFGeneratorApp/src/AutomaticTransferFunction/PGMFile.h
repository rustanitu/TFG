/// PGMFile.h
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#ifndef PGM_FILE_H
#define PGM_FILE_H

#include <fstream>

#define PGM_EXT ".pgm"
#define PGM_CODE "P5"

/// <summary>
/// This class implements an imagne file writer in a pgm
/// format. The file must be openned before any write
/// instruction and closed when there is nothing else
/// to write in it.
/// </summary>
class PGMFile
{
public:
  /// <summary>
  /// Initializes a new instance of the <see cref="PGMFile"/> class.
  /// </summary>
  /// <param name="path">The full path to the file.
  /// Extension isn't needed and it'll be considered part
  /// of the file's name, if passed</param>
  /// <param name="width">The image width.</param>
  /// <param name="height">The image height.</param>
  PGMFile(const char* path, unsigned int width, unsigned int height);
  
  /// <summary>
  /// Finalizes an instance of the <see cref="PGMFile"/> class.
  /// </summary>
  ~PGMFile();

  /// <summary>
  /// Opens the image file.
  /// </summary>
  /// <returns>Returns true if the file was successfully openned.
  /// Otherwise it returns false.</returns>
  bool Open();
  /// <summary>
  /// Writes a byte to the image. The file must be openned
  /// before.
  /// </summary>
  /// <param name="byte">The byte.</param>
  void WriteByte(unsigned char byte);
  /// <summary>
  /// Writes a byte sequence to the image. The file must be
  /// openned before.
  /// </summary>
  /// <param name="byte">The byte.</param>
  void WriteByte(const char* byte);
  /// <summary>
  /// Writes a end line to the image. The file must be openned
  /// before.
  /// </summary>
  void WriteEndLine();
  /// <summary>
  /// Closes the image file.
  /// </summary>
  void Close();

private:
  /// <summary>
  /// The file path.
  /// </summary>
  char* m_path;
  /// <summary>
  /// The image width.
  /// </summary>
  unsigned int m_width;
  /// <summary>
  /// The image height.
  /// </summary>
  unsigned int m_height;
  /// <summary>
  /// Reference to the file.
  /// </summary>
  FILE* m_file;
};

#endif