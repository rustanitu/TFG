/// TransferFunction.cpp
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#include "TransferFunction.h"

#include <assert.h>
#include <iostream>
#include <fstream>

/// <summary>
/// Initializes a new instance of the <see cref="TransferFunction"/> class.
/// </summary>
TransferFunction::TransferFunction(const char* path) : ITransferFunction(path, TF_EXT)
, m_value(NULL)
, m_distance(NULL)
, m_sigma(NULL)
{
  m_color_size = 0;
  for (int i = 0; i < MAX_V; ++i)
  {
    m_color[i * 3] = 0;
    m_color[(i * 3) + 1] = 0;
    m_color[(i * 3) + 2] = 0;
    m_has_color[i] = false;
  }
}

/// <summary>
/// Generates a transfer function file at a given path.
/// If a file with the same path already exists, it'll
/// be replaced.
/// </summary>
/// <returns>Returns true if the transfer function can
/// be generated, false otherwise.</returns>
bool TransferFunction::Generate()
{
  assert(m_value && m_distance && m_sigma && m_path);

  std::ofstream file;
  file.open(m_path);
  
  if (!file.is_open())
    return false;

  file << "linear" << "\n";
  file << "0" << "\n";
  file << m_color_size << "\n";

  // Assign color to transfer function
  for (int i = 0; i < MAX_V; ++i)
  {
    if (m_has_color[i])
    {
      float r = m_color[(i * 3)] / 255.0f;
      float g = m_color[(i * 3) + 1] / 255.0f;
      float b = m_color[(i * 3) + 2] / 255.0f;

      file << r << " ";
      file << g << " ";
      file << b << " ";
      file << i << std::endl;
    }
  }

  file << m_values_size << "\n";
  
  //  boundary center
  //         .
  //        / \       |
  //       / | \      |
  //      /  |  \     |
  //     /   |   \    | amax
  //    /    |    \   |
  //   /     |     \  |
  //  /      |      \ |
  //  ------ 0 ++++++ 
  // |---------------|
  //       base
  
  float amax = 1.0f;
  float base = 0.0f;

  // Assign opacity to transfer function
  for (int i = 0; i < m_values_size; ++i)
  {
    base = 15.0f * m_sigma[i];
    unsigned int value = (unsigned int)m_value[i];
    float x = m_distance[value];
    if (x >= -base && x <= base)
    {
      float a = 0.0f;
      if (x >= 0.0)
        a = -(amax * x) / base;
      else
        a = (amax * x) / base;

      a += amax;
      file << a << "\t" << value << std::endl;
    }
    else
      file << 0 << "\t" << value << std::endl;
  }

  file.close();
  return true;
}

/// <summary>
/// Sets the color associated to the intensity value.
/// </summary>
/// <param name="value">The intensity voxel value.</param>
/// <param name="red">The red color component.</param>
/// <param name="green">The green color component.</param>
/// <param name="blue">The blue color component.</param>
void TransferFunction::SetValueColor(unsigned char value, unsigned char red, unsigned char green, unsigned char blue)
{
  m_color_size++;
  m_has_color[value] = true;
  m_color[(value*3)] = red;
  m_color[(value*3) + 1] = green;
  m_color[(value*3) + 2] = blue;
}

/// <summary>
/// Specifies the distance between a intensity value
/// and its closest boundary. Thus, the input arrays'
/// size must range from 2 to 256. Any array content
/// whose index is greater than 255, it'll be ignored.
/// </summary>
/// <param name="values">The values array.</param>
/// <param name="distances">The distances to the closest boundaries.</param>
/// <param name="sigmas">The sigmas of the boundaries.</param>
/// <param name="n">The input arrays' size.</param>
void TransferFunction::SetClosestBoundaryDistances(unsigned char* values, float* distances, float* sigmas, int n)
{
  // At least 2 values are needed to interpolate the transfer funciton.
  assert(n >= 2 && n <= MAX_V);

  m_values_size = n;

  if (values)
    m_value = values;

  if (distances)
    m_distance = distances;

  if (sigmas)
    m_sigma = sigmas;
}