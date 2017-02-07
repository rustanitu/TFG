/// VolWriter.cpp
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#include <volrend\VolWriter.h>
#include <assert.h>
#include <string.h>
#include <math.h>

/// <summary>
/// Initializes a new instance of the <see cref="VolWriter"/> class.
/// </summary>
/// <param name="path">The full path to the file.
/// Extension isn't needed and it'll be considered part of the file's name</param>
/// <param name="width">The image width.</param>
/// <param name="height">The image height.</param>
VolWriter::VolWriter(const char* path, unsigned int width, unsigned int height, unsigned int depth)
: m_width(width)
, m_height(height)
, m_depth(depth)
, m_file(NULL)
{
	if (!path)
		throw std::exception_ptr();

	m_path = path;
	m_path.append(VOL_EXT);
}

VolWriter::VolWriter(const char* path, unsigned int size)
  : m_width(size)
  , m_height(size)
  , m_depth(size)
  , m_file(NULL)
{
  if (!path)
    throw std::exception_ptr();

  m_path = path;
  m_path.append(VOL_EXT);
}

/// <summary>
/// Finalizes an instance of the <see cref="VolWriter"/> class.
/// </summary>
VolWriter::~VolWriter()
{
	if (m_file)
		delete m_file;
	m_file = NULL;
}


/// <summary>
/// Opens the image file.
/// </summary>
/// <returns>Returns true if the file was successfully openned.
/// Otherwise it returns false.</returns>
bool VolWriter::Open()
{
	if (m_file)
		throw std::domain_error("The file is already opened!");

	int error = fopen_s(&m_file, m_path.c_str(), "wb");
	if (error)
	{
		printf("Erro ao tentar abrir o arquivo %s!\n", m_path);
		return false;
	}

	fprintf_s(m_file, "synthetic_model\n");
	int found = m_path.find_last_of('\\');
	std::string name = m_path.substr(found + 1, m_path.length());
	fprintf_s(m_file, "%s\n", name.c_str());
	fprintf_s(m_file, "0\n");
	fprintf_s(m_file, "%d %d %d\n", m_width, m_height, m_depth);
	return true;
}

void VolWriter::WriteSphere()
{
  double c = m_width / 2.0f;
  double q = m_width / 4.0f;
  for (int z = 0; z < m_depth; z++) {
    for (int y = 0; y < m_height; y++) {
      for (int x = 0; x < m_width; x++) {
        double dist = sqrt((c - x) * (c - x) + (c - y) * (c - y) + (c - z) * (c - z));
        if (dist > c) {
          Write(0);
          continue;
        }

        double p = c - dist;
        float f = 0.0f;
        if (p > q) {
          p -= q;
          f = 127.0f;
        }
        p = p / q;
        p = p * 4.0f;
        p -= 2.0f;
        f += 127.0f * (1 + erf(p)) * 0.5f;
        Write(f);
      }
    }
  }
}

void VolWriter::WriteQuads()
{
  float d = 4.0f;
  float vmin = 0.0f;
  float vmax = 255.0f;
  int restart_point = m_depth / 4;
  for (int z = 0; z < m_depth; z++) {
    float p = (d * z / (float)(m_depth - 1)) - (d * 0.5f);
    float f = vmin + (vmax - vmin) * (1 + erf(p)) * 0.5f;
    for (int y = 0; y < m_height; y++) {
      for (int x = 0; x < m_width; x++) {
        Write(f);
      }
    }
  }
}

void VolWriter::WriteLine()
{
  for (int z = 0; z < m_depth; z++) {
    for (int y = 0; y < m_height; y++) {
      for (int x = 0; x < m_width; x++) {
        if (x >= m_width / 2)
          Write(10.0f);
        else
          Write(5.0f);
      }
    }
  }
}

/// <summary>
/// Writes a byte to the image. The file must be openned
/// before.
/// </summary>
/// <param name="byte">The byte.</param>
void VolWriter::Write(int x, int y, int z, float value)
{
	fprintf_s(m_file, "%d %d %d %f\n", x, y, z, value);
}

void VolWriter::Write(const float& value)
{
	fprintf_s(m_file, "%f\n", value);
}

/// <summary>
/// Closes the image file.
/// </summary>
void VolWriter::Close()
{
	fclose(m_file);
	m_file = NULL;
}
