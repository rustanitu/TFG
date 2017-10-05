/// VolWriter.h
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#ifndef VOL_WRITER_H
#define VOL_WRITER_H

#include <fstream>

#define VOL_EXT ".vol"

/// <summary>
/// This class implements an volume file writer in a vol
/// format. The file must be openned before any write
/// instruction and closed when there is nothing else
/// to write in it.
/// </summary>
class VolWriter
{
public:
  /// <summary>
  /// Initializes a new instance of the <see cref="VolWriter"/> class.
  /// </summary>
  /// <param name="path">The full path to the file.
  /// Extension isn't needed and it'll be considered part
  /// of the file's name, if passed</param>
  /// <param name="width">The volume width.</param>
  /// <param name="height">The volume height.</param>
  VolWriter(const char* path, unsigned int width, unsigned int height, unsigned int depth);
  VolWriter(const char* path, unsigned int size);
  
  /// <summary>
  /// Finalizes an instance of the <see cref="VolWriter"/> class.
  /// </summary>
  ~VolWriter();

  /// <summary>
  /// Opens the volume file.
  /// </summary>
  /// <returns>Returns true if the file was successfully openned.
  /// Otherwise it returns false.</returns>
  bool Open();

  void WriteSphere();
  void WriteQuads();
  void WriteLine();

  void Write(int x, int y, int z, float value);
	void Write(const float& value);

  /// <summary>
  /// Closes the volume file.
  /// </summary>
  void Close();

private:
  /// <summary>
  /// The file path.
  /// </summary>
  std::string m_path;

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