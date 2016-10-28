#include "Tank.h"
#include "Cell.h"

#include <algorithm>
#include <fstream>

Tank::Tank()
: m_cells(NULL)
, m_vertices(NULL)
, m_current_timestep(0)
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

float Tank::GetValue(const UINT32& x, const UINT32& y, const UINT32& z)
{
	UINT32 xt = std::min(x, m_width - 1);
	UINT32 yt = std::min(y, m_height - 1);
	UINT32 zt = std::min(z, m_depth - 1);

	return GetValue (GetId (xt, yt, zt));
}

float Tank::GetValue(const UINT32& id)
{
	return m_cells[id].GetValue(m_current_timestep);
}

float Tank::CalculateGradient(const UINT32& x, const UINT32& y, const UINT32& z)
{
	if ( !m_cells[GetId(x, y, z)].IsActive() )
		return 0.0f;

	UINT32 id = GetId(x - 1, y, z);
	float lx = m_cells[id].IsActive() ? GetValue(id) : 0.0f;
	id = GetId(x + 1, y, z);
  float rx = m_cells[id].IsActive() ? GetValue(id) : 0.0f;
	float gx = rx - lx;

	id = GetId(x, y + 1, z);
  float uy = m_cells[id].IsActive() ? GetValue(id) : 0.0f;
	id = GetId(x, y - 1, z);
  float dy = m_cells[id].IsActive() ? GetValue(id) : 0.0f;
	float gy = uy - dy;

  id = GetId(x, y, z + 1);
  float fz = m_cells[id].IsActive() ? GetValue(id) : 0.0f;
  id = GetId(x, y, z - 1);
  float bz = m_cells[id].IsActive() ? GetValue(id) : 0.0f;
	float gz = fz - bz;

	float g = sqrt(gx*gx + gy*gy + gz*gz);

	g = fmax(0.0f, g);
	m_max_gradient = fmax(m_max_gradient, g);

	return g;
}

float Tank::CalculateLaplacian(const UINT32& x, const UINT32& y, const UINT32& z)
{
  if (!m_cells[GetId(x, y, z)].IsActive())
    return 0.0f;

  float vv = 2 * GetValue(x, y, z);

  UINT32 id = GetId(x - 1, y, z);
  float lx = m_cells[id].IsActive() ? GetValue(id) : 0.0f;
  id = GetId(x + 1, y, z);
  float rx = m_cells[id].IsActive() ? GetValue(id) : 0.0f;
  lx = rx - vv + lx;

  id = GetId(x, y + 1, z);
  float uy = m_cells[id].IsActive() ? GetValue(id) : 0.0f;
  id = GetId(x, y - 1, z);
  float dy = m_cells[id].IsActive() ? GetValue(id) : 0.0f;
  float ly = uy - vv + dy;

  id = GetId(x, y, z + 1);
  float fz = m_cells[id].IsActive() ? GetValue(id) : 0.0f;
  id = GetId(x, y, z - 1);
  float bz = m_cells[id].IsActive() ? GetValue(id) : 0.0f;
  float lz = fz - vv + bz;

	float l = lx + ly + lz;

	m_max_laplacian = fmax(m_max_laplacian, l);
	m_min_laplacian = fmin(m_min_laplacian, l);

	return l;
}