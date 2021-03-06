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
  delete [] m_value;
  delete [] m_distance;
}

float TransferFunction::CenteredTriangleFunction(float max, float base, float x)
{
  //  boundary center
  //         .
  //        / \       |
  //       / | \      |
  //      /  |  \     |
  //     /   |   \    | max
  //    /    |    \   |
  //   /     |     \  |
  //  /      |      \ |
  //  ------ 0 ++++++ 
  // |-------|-------|
  //       base

  float a = 0.0f;
  if (x >= -base && x <= base) {
    if (x >= 0.0)
      a = -(max * x) / base;
    else
      a = (max * x) / base;

    a += max;
  }
  return a;
}

float TransferFunction::GetDistance(unsigned char v)
{
  assert(m_distance);
  return m_distance[v];
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

  if (!file.is_open())
    return false;

  file << "linear" << std::endl;
  file << "0" << std::endl;

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
  IupSetAttribute(m_bx_plot, "CLEAR", "YES");
  IupPlotBegin(m_bx_plot, 0);
  
  float amax = 0.4f;
  float base = m_thickness;

  // Assign opacity to transfer function
  int b = 0;
  float last_a = 0.0f;
  for (int i = 0; i < m_values_size; ++i)
  {
    unsigned int value = (unsigned int)m_value[i];
    float x = m_distance[value];

    IupPlotAdd(m_bx_plot, value, fmax(fmin(x, 20), -20));
    
    float a = CenteredTriangleFunction(amax, base, x);

    if (a != last_a && last_a == 0.0f)
      b++;

    last_a = a;
    if (m_boundary != 0 && b != m_boundary)
      a = 0.0f;
    
    file << a << "\t" << value << std::endl;
    IupPlotAdd(m_tf_plot, value, a);
  }

  IupPlotEnd(m_tf_plot);
  IupSetAttribute(m_tf_plot, "DS_NAME", "Transfer Function");
  IupSetAttribute(m_tf_plot, "DS_COLOR", "128 128 128");
  IupSetAttribute(m_tf_plot, "REDRAW", "YES");

  IupPlotEnd(m_bx_plot);
  IupSetAttribute(m_bx_plot, "DS_NAME", "p(v)");

  IupPlotBegin(m_bx_plot, 0);
  IupPlotAdd(m_bx_plot, 0, 0);
  IupPlotAdd(m_bx_plot, 255, 0);
  IupPlotEnd(m_bx_plot);
  IupSetAttribute(m_bx_plot, "DS_NAME", "0");
  IupSetAttribute(m_bx_plot, "DS_COLOR", "0 0 0");

  //IupPlotBegin(m_bx_plot, 0);
  //IupPlotAdd(m_bx_plot, 0, m_thickness);
  //IupPlotAdd(m_bx_plot, 255, m_thickness);
  //IupPlotEnd(m_bx_plot);
  //IupSetAttribute(m_bx_plot, "DS_NAME", "b(x)");
  //IupSetAttribute(m_bx_plot, "DS_COLOR", "128 128 0");

  //IupPlotBegin(m_bx_plot, 0);
  //IupPlotAdd(m_bx_plot, 0, -m_thickness);
  //IupPlotAdd(m_bx_plot, 255, -m_thickness);
  //IupPlotEnd(m_bx_plot);
  //IupSetAttribute(m_bx_plot, "DS_NAME", "b(x)");
  //IupSetAttribute(m_bx_plot, "DS_COLOR", "128 128 0");

  IupSetAttribute(m_bx_plot, "REDRAW", "YES");

  file.close();
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