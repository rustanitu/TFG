/// RAWFile.cpp
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#include "RAWFile.h"
#include <assert.h>
#include <string.h>

/// <summary>
/// Initializes a new instance of the <see cref="RAWFile"/> class.
/// </summary>
/// <param name="path">The full path to the file.
/// Extension isn't needed and it'll be considered part of the file's name</param>
/// <param name="width">The image width.</param>
/// <param name="height">The image height.</param>
RAWFile::RAWFile(const char* path, unsigned int width, unsigned int height, unsigned int depth)
: m_width(width)
, m_height(height)
, m_depth(depth)
, m_file(NULL)
{
  if (!path)
    throw std::exception_ptr();

  char ext[15];
  sprintf_s(ext, ".1.%dx%dx%d", width, height, depth);

  int path_len = strlen(path);
  int length = path_len + strlen(RAW_EXT) + strlen(ext);
  m_path = new char[length + 1];
  m_path = strcpy(m_path, path);
  strcpy(m_path + path_len, ext);
  strcpy(m_path + path_len + strlen(ext), RAW_EXT);
  m_path[length] = '\0';
}


/// <summary>
/// Finalizes an instance of the <see cref="RAWFile"/> class.
/// </summary>
RAWFile::~RAWFile()
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
bool RAWFile::Open()
{
  if (m_file)
    throw std::domain_error("The file is already opened!");

  int error = fopen_s(&m_file, m_path, "wb");
  if (error)
  {
    printf("Erro ao tentar abrir o arquivo %s!\n", m_path);
    return false;
  }

	int vmin = 115;
	int vmax = 2*vmin;
	float erf[17] = {0.9953f, 0.9866f, 0.9661f, 0.9229f, 0.8427f, 0.7111f, 0.5205f, 0.2763f, 0.0f, -0.2763f, -0.5205f, -0.7111f, -0.8427f, -0.9229f, -0.9661f, -0.9866f, -0.9953f};
	//float erf[9] = {0.9953f, 0.9661f, 0.8427f, 0.5205f, 0.0f, -0.5205f, -0.8427f, -0.9661f, -0.9953f};
	//float erf[5] = {0.9953f, 0.8427f, 0.0f, -0.8427f, -0.9953f};

  int c = m_width / 2;
	int q = m_width / 4;
  for (int z = 0; z < m_depth; z++) {
    for (int y = 0; y < m_height; y++) {
      for (int x = 0; x < m_width; x++) {
				int p = sqrt((c - x) * (c - x) + (c - y) * (c - y) + (c - z) * (c - z)) - q;
				if ( p < 0 )
					p = 0;
				else if ( p > 16 )
					p = 16;
				int v = vmin + (vmax - vmin) * erf[p];
        WriteByte(v);
      }
    }
  }
  return true;
}

/// <summary>
/// Writes a byte to the image. The file must be openned
/// before.
/// </summary>
/// <param name="byte">The byte.</param>
void RAWFile::WriteByte(unsigned char byte)
{
  fprintf_s(m_file, "%c", byte);
}

/// <summary>
/// Writes a byte to the image. The file must be openned
/// before.
/// </summary>
/// <param name="byte">The byte.</param>
void RAWFile::WriteByte(const char* byte)
{
  fprintf_s(m_file, "%s", byte);
}

/// <summary>
/// Writes a end line to the image. The file must be openned
/// before.
/// </summary>
void RAWFile::WriteEndLine()
{
  fprintf_s(m_file, "\n");
}

/// <summary>
/// Closes the image file.
/// </summary>
void RAWFile::Close()
{
  fclose(m_file);
  m_file = NULL;
}
