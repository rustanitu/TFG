/// PGMFile.cpp
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#include "PGMFile.h"
#include <assert.h>
#include <string.h>

/// <summary>
/// Initializes a new instance of the <see cref="PGMFile"/> class.
/// </summary>
/// <param name="path">The full path to the file.
/// Extension isn't needed and it'll be considered part of the file's name</param>
/// <param name="width">The image width.</param>
/// <param name="height">The image height.</param>
PGMFile::PGMFile(const char* path, unsigned int width, unsigned int height)
: m_width(width)
, m_height(height)
, m_file(NULL)
{
  if (!path)
    throw std::exception_ptr();

  int path_len = strlen(path);
  int length = path_len + strlen(PGM_EXT);
  m_path = new char[length + 1];
  m_path = strcpy(m_path, path);
  strcpy(m_path + path_len, PGM_EXT);
  m_path[length] = '\0';
}


/// <summary>
/// Finalizes an instance of the <see cref="PGMFile"/> class.
/// </summary>
PGMFile::~PGMFile()
{
  if (m_path)
    delete m_path;
  m_path = NULL;

  if (m_file)
    delete m_file;
  m_file = NULL;
}


/// <summary>
/// Opens the image file.
/// </summary>
/// <returns>Returns true if the file was successfully openned.
/// Otherwise it returns false.</returns>
bool PGMFile::Open()
{
  if (m_file)
    throw std::domain_error("The file is already opened!");

  int error = fopen_s(&m_file, m_path, "wb");
  if (error)
  {
    printf("Erro ao tentar abrir o arquivo %s%s!\n", m_path, PGM_EXT);
    return false;
  }

  // PGM Header File
  WriteByte(PGM_CODE);
  WriteEndLine();
  WriteByte(m_width);
  WriteByte(m_height);
  WriteEndLine();
  WriteByte(UCHAR_MAX);
  WriteEndLine();

  return true;
}

/// <summary>
/// Writes a byte to the image. The file must be openned
/// before.
/// </summary>
/// <param name="byte">The byte.</param>
void PGMFile::WriteByte(unsigned int byte)
{
  fprintf_s(m_file, "%d ", byte);
}

/// <summary>
/// Writes a byte to the image. The file must be openned
/// before.
/// </summary>
/// <param name="byte">The byte.</param>
void PGMFile::WriteByte(const char* byte)
{
  fprintf_s(m_file, "%s", byte);
}

/// <summary>
/// Writes a end line to the image. The file must be openned
/// before.
/// </summary>
void PGMFile::WriteEndLine()
{
  fprintf_s(m_file, "\n");
}

/// <summary>
/// Closes the image file.
/// </summary>
void PGMFile::Close()
{
  fclose(m_file);
  m_file = NULL;
}
