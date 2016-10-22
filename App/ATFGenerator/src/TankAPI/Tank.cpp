#include "Tank.h"
#include "Cell.h"

#include <fstream>

Tank::Tank()
: m_cells(NULL)
, m_vertices(NULL)
{
}

Tank::~Tank()
{
  Clear();
}

void Tank::Clear()
{
  m_steps.clear();
  
  delete[] m_cells;
  m_cells = NULL;
  
  delete[] m_vertices;
  m_vertices = NULL;
}

bool Tank::Read(const char* filepath)
{
  // It opens the tank file
  std::ifstream file(filepath);
  if (!file || !file.is_open())
    return false;

  // It reads file's header
  int ni, nj, nk, nactive;
  if (!(file >> ni >> nj >> nk >> nactive >> m_nvertices >> m_nsteps))
    return false;

	m_width = ni;
	m_height = nj;
	m_depth = nk;

  m_ncells = ni * nj * nk;
  m_cells = new Cell[m_ncells];
  if (!m_cells)
    return false;

  m_vertices = new float[m_nvertices * 3];
  if (!m_vertices)
    return false;

  // It gets all timesteps
  for (int step = 0; step < m_nsteps; ++step)
  {
    // sizeof("dd/mm/aaaa") = 11
    char timestep[11];
    int dd, mm, aaaa;
    char b;
    if (!(file >> dd >> b >> mm >> b >> aaaa))
      return false;

    sprintf_s(timestep, "%d/%d/%d", dd, mm, aaaa);
    m_steps.push_back(timestep);
  }

  // It gets all vertices
  for (int i = 0; i < m_nvertices; ++i)
  {
    float x, y, z;
    if (!(file >> x >> y >> z))
      return false;

    int baseidx = i * 3;
    m_vertices[baseidx] = x;
    m_vertices[baseidx + 1] = y;
    m_vertices[baseidx + 2] = z;
  }

  // It gets all cells
  for (int c = 0; c < m_ncells; ++c)
  {
    bool active;
    int i, j, k;
    if (!(file >> active >> i >> j >> k))
      return false;

		int id = GetId (i, j, k);
    Cell* cell = &(m_cells[id]);
    cell->Init(i, j, k, active, m_nsteps);

    // It sets the index of the ith vertex
    for (int v = 0; v < 8; ++v)
    {
      int index;
      if (!(file >> index))
        return false;

      cell->SetIthVertexIndex(v, index);
    }

    // It sets the index of the adjacent ith cell
    for (int a = 0; a < 6; ++a)
    {
      int index;
      if (!(file >> index))
        return false;

      cell->SetAdjcentCellIndex(a, index);
    }

    // It sets the cell value of the p timestamp
    for (int t = 0; t < m_nsteps; ++t)
    {
      float value;
      if (!(file >> value))
        return false;
      
      cell->SetValue(t, value);
    }
  }

  return true;
}

int Tank::GetValue(const UINT32& x, const UINT32& y, const UINT32& z)
{
	return GetValue (GetId (x, y, z));
}

int Tank::GetValue (float x, float y, float z)
{
	return GetValue(GetId (x, y, z));
}

int Tank::GetValue(const UINT32& id)
{
	return m_cells[id].GetValue (0);
}