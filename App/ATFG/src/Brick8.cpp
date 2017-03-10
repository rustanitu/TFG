/// Brick8.cpp
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#include "Brick8.h"

Brick8::Brick8(vr::ScalarField* scalarfield)
: m_nodes(NULL)
, m_width(scalarfield->GetWidth() + 1)
, m_height(scalarfield->GetHeight() + 1)
, m_depth(scalarfield->GetDepth() + 1)
{
  int size = m_width * m_height * m_depth;
  m_nodes = new Node[size];
  if (scalarfield)
  {
    for (int x = 0; x < scalarfield->GetWidth(); ++x)
    {
      for (int y = 0; y < scalarfield->GetHeight(); ++y)
      {
        for (int z = 0; z < scalarfield->GetDepth(); ++z)
        {
          Assign(scalarfield->GetValue(x, y, z), x, y, z);
        }
      }
    }

    for (int i = 0; i < size; ++i)
    {
      m_nodes[i].value /= m_nodes[i].refcount;
    }
  }
}

Brick8::~Brick8()
{
  delete[] m_nodes;
}

int Brick8::GetId(int x, int y, int z)
{
  return x + y * m_width + z * m_width * m_height;
}

void Brick8::Assign(double val, int x, int y, int z)
{
  m_nodes[GetId(x + 1, y, z)].value = val;
  m_nodes[GetId(x + 1, y, z)].refcount++;
  m_nodes[GetId(x + 1, y + 1, z)].value = val;
  m_nodes[GetId(x + 1, y + 1, z)].refcount++;
  m_nodes[GetId(x + 1, y + 1, z + 1)].value = val;
  m_nodes[GetId(x + 1, y + 1, z + 1)].refcount++;
  m_nodes[GetId(x + 1, y, z + 1)].value = val;
  m_nodes[GetId(x + 1, y, z + 1)].refcount++;
  m_nodes[GetId(x, y, z)].value = val;
  m_nodes[GetId(x, y, z)].refcount++;
  m_nodes[GetId(x, y + 1, z)].value = val;
  m_nodes[GetId(x, y + 1, z)].refcount++;
  m_nodes[GetId(x, y + 1, z + 1)].value = val;
  m_nodes[GetId(x, y + 1, z + 1)].refcount++;
  m_nodes[GetId(x, y, z + 1)].value = val;
  m_nodes[GetId(x, y, z + 1)].refcount++;
}