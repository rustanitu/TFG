#include "Cell.h"

#include <stdlib.h>

Cell::Cell()
: m_i(-1)
, m_j(-1)
, m_k(-1)
, m_active(false)
, m_nsteps(-1)
, m_values(NULL)
{
}

void Cell::Init(const int& i, const int& j, const int& k, const bool& active, const int& nsteps)
{
  m_i = i;
  m_j = j;
  m_k = k;
  m_active = active;
  m_nsteps = nsteps;
  m_values = new int[nsteps];
}

int Cell::GetIthVertexIndex(const int& i)
{
  if (i < 0 || i > 7)
    return -1;
  return m_vertex_index[i];
}

int Cell::GetAdjcentCellIndex(const int& face)
{
  if (face < 0 || face > 5)
    return -1;
  return m_adjcell_index[face];
}

int Cell::GetValue(const int& step)
{
  if (step < 0 || step > m_nsteps - 1)
    return -1;
  return m_values[step];
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

bool Cell::SetValue(const int& step, const int& value)
{
  if (step < 0 || step > m_nsteps - 1)
    return false;

  m_values[step] = value;
  return true;
}