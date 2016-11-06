/// VolWriter.cpp
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#include <volrend\VolWriter.h>
#include <assert.h>
#include <string.h>

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

	int vmin = 0;
	int vmax = 255;
	int vmin2 = vmax;
	int vmax2 = vmin2 + vmax - vmin;
	int size = 17;
	int halfsize = size / 2;
	float erf[17] = {-0.9953f, -0.9866f, -0.9661f, -0.9229f, -0.8427f, -0.7111f, -0.5205f, -0.2763f, 0.0f, 0.2763f, 0.5205f, 0.7111f, 0.8427f, 0.9229f, 0.9661f, 0.9866f, 0.9953f};

	int c = m_width / 2;
	int maxdist = sqrt(3 * c * c);
	int c1 = (maxdist / 2) - halfsize;
	int c2 = (3 * maxdist / 4) - halfsize;
	for (int z = 0; z < m_depth; z++) {
		for (int y = 0; y < m_height; y++) {
			for (int x = 0; x < m_width; x++) {
				int p = abs(sqrt((c - x) * (c - x) + (c - y) * (c - y) + (c - z) * (c - z)) - maxdist);
				float v = vmin + (vmax - vmin) * (1 + erf[0]) * 0.5f;
				if ( p >= c1 && p - c1 < size )
					v = vmin + (vmax - vmin) * (1 + erf[p - c1]) * 0.5f;
#if 1 // One Sphere
				else if (p >= c1 )
					v = vmin + (vmax - vmin) * (1 + erf[size - 1]) * 0.5f;
#else // Two Spheres
				else if ( p >= c2 && p - c2 < size)
					v = vmin2 + (vmax2 - vmin2) * (1 + erf[p - c2]) * 0.5f;
				else if ( p >= c1 && p <= c2 )
					v = vmin2 + (vmax2 - vmin2) * (1 + erf[0]) * 0.5f;
				else if (p > c2 )
					v = vmin2 + (vmax2 - vmin2) * (1 + erf[size - 1]) * 0.5f;
#endif
				Write(v);
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
