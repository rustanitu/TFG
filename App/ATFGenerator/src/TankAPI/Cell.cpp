#include "Cell.h"

#include <stdlib.h>

Cell::Cell()
: m_i(-1)
, m_j(-1)
, m_k(-1)
, m_active(false)
, m_center(-FLT_MAX)
{
}

void Cell::Init(const int& i, const int& j, const int& k, const bool& active, const int& nsteps)
{
  m_i = i;
  m_j = j;
  m_k = k;
  m_active = active;
}

int Cell::GetIthVertexIndex(const int& i) const
{
  if (i < 0 || i > 7)
    return -1;
  return m_vertex_index[i];
}

int Cell::GetAdjcentCellIndex(const int& face) const
{
  if (face < 0 || face > 5)
    return -1;
  return m_adjcell_index[face];
}

double Cell::GetValue(const int& step) const
{
  //if (step < 0 || step > m_nsteps - 1)
  //  return -1;
  //return m_values[step];
  return m_value;
}

bool Cell::SetIthVertexIndex(const int& ith_vertex, const int& index)
{
  if (ith_vertex < 0 || ith_vertex > 7)
    return false;

  m_vertex_index[ith_vertex] = index;
  return true;
}

bool Cell::SetAdjcentCellIndex(const int& face, const int& index)
{
  if (face < 0 || face > 5)
    return false;

  m_adjcell_index[face] = index;
  return true;
}

bool Cell::SetValue(const int& step, const double& value)
{
  //if (step < 0 || step > m_nsteps - 1)
  //  return false;
  //
  //m_values[step] = value;
  m_value = value;
  return true;
}

int* Cell::GetFaceVertices(const int& face) const
{
  int* vertices = new int[4];
  switch (face) {
    case 0:
      vertices[0] = 0;
      vertices[1] = 1;
      vertices[2] = 3;
      vertices[3] = 2;
      break;
    case 1:
      vertices[0] = 4;
      vertices[1] = 6;
      vertices[2] = 7;
      vertices[3] = 5;
      break;
    case 2:
      vertices[0] = 0;
      vertices[1] = 2;
      vertices[2] = 6;
      vertices[3] = 4;
      break;
    case 3:
      vertices[0] = 1;
      vertices[1] = 5;
      vertices[2] = 7;
      vertices[3] = 3;
      break;
    case 4:
      vertices[0] = 0;
      vertices[1] = 4;
      vertices[2] = 5;
      vertices[3] = 1;
      break;
    case 5:
      vertices[0] = 2;
      vertices[1] = 3;
      vertices[2] = 7;
      vertices[3] = 6;
      break;
    default:
      return NULL;
  }
  return vertices;
}