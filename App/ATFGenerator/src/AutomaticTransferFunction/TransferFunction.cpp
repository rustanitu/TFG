/// TransferFunction.cpp
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#include "TransferFunction.h"

#include <assert.h>
#include <iostream>
#include <fstream>
#include <iup_plot.h>

/// <summary>
/// Initializes a new instance of the <see cref="TransferFunction"/> class.
/// </summary>
TransferFunction::TransferFunction(const char* path) : ITransferFunction(path, TF_EXT)
, m_value(NULL)
, m_distance(NULL)
, m_thickness(1.0f)
, m_boundary(0)
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
/// Finalizes an instance of the <see cref="TransferFunction"/> class.
/// </summary>
TransferFunction::~TransferFunction()
{
  free(m_value);
  free(m_distance);
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
  if (!m_value || !m_distance || !m_path)
    throw std::exception_ptr();

  std::ofstream file;
  file.open(m_path);

  int len = strlen(m_path);
  char* csv_name = new char[len];
  csv_name = (char*)memcpy(csv_name, m_path, len);
  csv_name[--len] = '\0';
  csv_name[--len] = 'v';
  csv_name[--len] = 's';
  csv_name[--len] = 'c';
  std::ofstream csv;
  csv.open(csv_name);
  
  if (!file.is_open() || !csv.is_open())
    return false;

  file << "linear" << std::endl;
  file << "0" << std::endl;
  csv << "; Alpha" << std::endl;

  if (m_color_size < 2)
    throw std::domain_error("At least two color must be set!");

  file << m_color_size << std::endl;

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

  file << m_values_size << std::endl;

  IupSetAttribute(m_tf_plot, "CLEAR", "YES");
  IupPlotBegin(m_tf_plot, 0);
  
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
  // |-------|-------|
  //       base
  
  float amax = 1.0f;
  float base = m_thickness;

  int b = 0;
  float last_a = 0.0f;

  // Assign opacity to transfer function
  for (int i = 0; i < m_values_size; ++i)
  {
    unsigned int value = (unsigned int)m_value[i];
    float x = m_distance[value];
    
    float a = 0.0f;
    if (x >= -base && x <= base)
    {
      if (x >= 0.0)
        a = -(amax * x) / base;
      else
        a = (amax * x) / base;

      a += amax;
    }

    if (a != last_a && last_a == 0.0f)
      b++;

    last_a = a;
    if (m_boundary != 0 && b != m_boundary)
      a = 0.0f;
    
    file << a << "\t" << value << std::endl;
    IupPlotAdd(m_tf_plot, value, a);

    int ipart = a;
    int fpart = (a - ipart) * 1000;
    csv << value << "; " << ipart << "," << fpart << std::endl;
  }

  IupPlotEnd(m_tf_plot);
  IupSetAttribute(m_tf_plot, "DS_NAME", "Transfer Function");
  IupSetAttribute(m_tf_plot, "REDRAW", "YES");

  file.close();
  csv.close();
  return true;
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
void TransferFunction::SetClosestBoundaryDistances(unsigned char* values, float* distances, int n)
{
  if (n < 2 || n > MAX_V)
    throw std::length_error("At least 2 values are needed to interpolate the transfer funciton!");

  m_values_size = n;

  delete [] m_value;
  delete [] m_distance;

  if (values)
    m_value = values;

  if (distances)
    m_distance = distances;
}