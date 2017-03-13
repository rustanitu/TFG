#include "Tank.h"

#include <algorithm>
#include <fstream>

//#define HESSIAN

Tank::Tank()
: m_cells(NULL)
, m_vertices(NULL)
, m_current_timestep(0)
, m_grad(NULL)
, m_hess(NULL)
{
	printf("Tank criado.\n");
}

Tank::~Tank()
{
	printf("Tank destruido.\n");
	m_steps.clear();

	delete[] m_cells;
	delete[] m_vertices;
	delete[] m_grad;
	delete[] m_hess;
}

bool Tank::Read(const char* filepath)
{
	printf("Inicio da leitura do tank.\n");
	// It opens the tank file
	std::ifstream file(filepath);
	if ( !file || !file.is_open() )
		return false;

	// It reads file's header
	int ni, nj, nk, nactive;
	if ( !(file >> ni >> nj >> nk >> nactive >> m_nvertices >> m_nsteps) )
		return false;

	m_width = ni;
	m_height = nj;
	m_depth = nk;

	printf("Tamanho do Tank:\n");
	printf(" - tWidth: %d\n", m_width);
	printf(" - tHeight: %d\n", m_height);
	printf(" - tDepth: %d\n", m_depth);

	m_ncells = ni * nj * nk;
	m_cells = new Cell[m_ncells];
  m_grad = new glm::vec3[m_ncells];
  m_hess = new glm::mat3[m_ncells];
  if (!m_cells || !m_grad || !m_hess)
		return false;

	m_vertices = new glm::vec3[m_nvertices];
	if ( !m_vertices )
		return false;

	// It gets all timesteps
	for ( int step = 0; step < m_nsteps; ++step )
	{
		// sizeof("dd/mm/aaaa") = 11
		char timestep[11];
		int dd, mm, aaaa;
		char b;
		if ( !(file >> dd >> b >> mm >> b >> aaaa) )
			return false;

		sprintf_s(timestep, "%d/%d/%d", dd, mm, aaaa);
		m_steps.push_back(timestep);
	}

	// It gets all vertices
	for ( int i = 0; i < m_nvertices; ++i )
	{
		float x, y, z;
		if ( !(file >> x >> y >> z) )
			return false;

		m_vertices[i].x = x;
		m_vertices[i].y = y;
    m_vertices[i].z = z;
	}

	// It gets all cells
	for ( int c = 0; c < m_ncells; ++c )
	{
		bool active;
		int i, j, k;
		if ( !(file >> active >> i >> j >> k) )
			return false;

		int id = GetId(i, j, k);
    if (id != c)
    {
      printf("O mapeamento do volume difere do experado.\n");
      return false;
    }

		Cell* cell = &(m_cells[id]);
		cell->Init(i, j, k, active, m_nsteps);

		// It sets the index of the ith vertex
		for ( int v = 0; v < 8; ++v )
		{
			int index;
			if ( !(file >> index) )
				return false;

			cell->SetIthVertexIndex(v, index);
		}

		int siblids[6] = {
			GetId(i + 1, j, k), // face 3
			GetId(i - 1, j, k), // face 2
			GetId(i, j + 1, k), // face 5
			GetId(i, j - 1, k), // face 4
			GetId(i, j, k + 1), // face 1
			GetId(i, j, k - 1), // face 0
		};

		// It sets the index of the adjacent ith cell
		for ( int a = 0; a < 6; ++a )
		{
			int index;
			if ( !(file >> index) )
				return false;

			if (index != -1) {
				bool match = false;
				for (int b = 0; b < 6; ++b)
        {
					if (index == siblids[b])
          {
						match = true;
						break;
					}
				}
				if (!match)
				{
					printf("O mapeamento do volume difere do experado.\n");
					delete[] m_cells;
					delete[] m_grad;
					delete[] m_hess;
					delete[] m_vertices;
					return false;
				}
			}

			cell->SetAdjcentCellIndex(a, index);
		}

		FillCellAdjCenter(*cell);

		// It sets the cell value of the p timestamp
		for ( int t = 0; t < m_nsteps; ++t )
		{
			double value;
			if ( !(file >> value) )
				return false;

			cell->SetValue(t, value);
			if ( active )
			{
				m_max_value = fmax(m_max_value, value);
				m_min_value = fmin(m_min_value, value);
			}
			else {
				cell->SetValue(t, 0.0f);
			}
		}
	}

	printf("Fim da leitura do tank.\n");

	return true;
}

double Tank::GetValue(const UINT32& x, const UINT32& y, const UINT32& z)
{
	if ( IsOutOfBoundary(x, y, z) )
		return -DBL_MAX;

	return GetValue(GetId(x, y, z));
}

double Tank::GetValue(const UINT32& id)
{
	return m_cells[id].GetValue(m_current_timestep);
	return m_cells[id].IsActive() ? m_cells[id].GetValue(m_current_timestep) : -DBL_MAX;
}

const Cell& Tank::GetCell(const UINT32& x, const UINT32& y, const UINT32& z)
{
  return GetCell(GetId(x, y, z));
}

const Cell& Tank::GetCell(const UINT32& id)
{
  return m_cells[id];
}

bool Tank::GetSegmentIntersection(const glm::vec3& k, const glm::vec3& l, const glm::vec3& m, const glm::vec3& n, float* s, float* t)
{
	double det;
  bool ok = false;

	if (((k.x != l.x) || (k.y != l.y)) &&
		((m.x != n.x) || (m.y != n.y)))  /* se nao e' paralela ao plano XY*/
	{
		det = (n.x - m.x) * (l.y - k.y) - (n.y - m.y) * (l.x - k.x);

    if (det == 0.0)
      ok = false;
    else
      ok = true;

		*s = ((n.x - m.x) * (m.y - k.y) - (n.y - m.y) * (m.x - k.x)) / det;
		*t = ((l.x - k.x) * (m.y - k.y) - (l.y - k.y) * (m.x - k.x)) / det;

		if (ok) return true;
	}

	if (((k.x != l.x) || (k.z != l.z)) &&
		((m.x != n.x) || (m.z != n.z)))  /* se nao e' paralela ao plano XZ*/
	{
		det = (n.x - m.x) * (l.z - k.z) - (n.z - m.z) * (l.x - k.x);

    if (det == 0.0)
      ok = false;
    else
      ok = true;

		*s = ((n.x - m.x) * (m.z - k.z) - (n.z - m.z) * (m.x - k.x)) / det;
		*t = ((l.x - k.x) * (m.z - k.z) - (l.z - k.z) * (m.x - k.x)) / det;

    if (ok) return true;
	}

	if (((k.y != l.y) || (k.z != l.z)) &&
		((m.y != n.y) || (m.z != n.z)))  /* se nao e' paralela ao plano YZ*/
	{
		det = (n.y - m.y) * (l.z - k.z) - (n.z - m.z) * (l.y - k.y);

    if (det == 0.0)
      ok = false;
    else
      ok = true;

		*s = ((n.y - m.y) * (m.z - k.z) - (n.z - m.z) * (m.y - k.y)) / det;
		*t = ((l.y - k.y) * (m.z - k.z) - (l.z - k.z) * (m.y - k.y)) / det;

    if (ok) return true;
	}

	return ok;
}

void Tank::FillCellAdjCenter(Cell& cell)
{
	//midle edge definition
	/*                                   e2
   *                  v2              <--                 v3
   *                  n2----------------------------------n3
   *                  /.                                  /| 
   *                 / .                                 / |
   *                /  .                                /  |
   *               /   .                               /   |
   *        e3 /  /    .         f0                ^  /    |
   *          v  /     .                       e1 /  /     |    
   *            /      . |e10                       /      |   | e9
   *           /       . v              f5         /       |   v
   *          /        .                          /        |
   *        n0----------------------------------n1         |
   *        v0         .      -->               v1         |
   *         |         .      e0                 |         |
   *         |   f3    .              e6         |    f2   |
   *         |         . /           <--         |         |
   *         |        n6 . . . . . . . . . . . . | . . . .n7
   *         |       .v6                         |        v7
   *   e11|  |      .       f4             e8 |  |       /
   *      v  | e7/ .                          v  |      /
   *         |  v .                              |     /   ^
   *         |   .               f1              |    /   /e5 
   *         |  .                                |   /
   *         | .                                 |  /
   *         |.                                  | /
   *        n4-----------------------------------n5
   *        v4                -->                v5
   *                          e4
   *        Kdir up ^ 
   *                |  ^ J regular
   *                | /
   *   I inverted   |/    
   *          <-----* ----->  
   *               /|      I regular
   *              / |
   * J inverted  v  |
   *                |
   *      Kdir down v
   */

	glm::vec3 uf = (m_vertices[cell.GetIthVertexIndex(2)] + m_vertices[cell.GetIthVertexIndex(3)]) / 2.0f;
	glm::vec3 df = (m_vertices[cell.GetIthVertexIndex(0)] + m_vertices[cell.GetIthVertexIndex(1)]) / 2.0f;
	glm::vec3 rf = (m_vertices[cell.GetIthVertexIndex(1)] + m_vertices[cell.GetIthVertexIndex(3)]) / 2.0f;
	glm::vec3 lf = (m_vertices[cell.GetIthVertexIndex(0)] + m_vertices[cell.GetIthVertexIndex(2)]) / 2.0f;

	glm::vec3 ul = (m_vertices[cell.GetIthVertexIndex(2)] + m_vertices[cell.GetIthVertexIndex(6)]) / 2.0f;
	glm::vec3 ur = (m_vertices[cell.GetIthVertexIndex(3)] + m_vertices[cell.GetIthVertexIndex(7)]) / 2.0f;
	glm::vec3 dr = (m_vertices[cell.GetIthVertexIndex(1)] + m_vertices[cell.GetIthVertexIndex(5)]) / 2.0f;
	glm::vec3 dl = (m_vertices[cell.GetIthVertexIndex(0)] + m_vertices[cell.GetIthVertexIndex(4)]) / 2.0f;

	glm::vec3 ub = (m_vertices[cell.GetIthVertexIndex(6)] + m_vertices[cell.GetIthVertexIndex(7)]) / 2.0f;
	glm::vec3 db = (m_vertices[cell.GetIthVertexIndex(4)] + m_vertices[cell.GetIthVertexIndex(5)]) / 2.0f;
	glm::vec3 rb = (m_vertices[cell.GetIthVertexIndex(5)] + m_vertices[cell.GetIthVertexIndex(7)]) / 2.0f;
	glm::vec3 lb = (m_vertices[cell.GetIthVertexIndex(4)] + m_vertices[cell.GetIthVertexIndex(6)]) / 2.0f;
	
	// Center face definition
	float psega = -1;
	float psegb = -1;

	glm::vec3 k_up_center(0);
	if (GetSegmentIntersection(uf, df, rf, lf, &psega, &psegb))
    k_up_center = uf + psega * (df - uf);
  cell.SetFaceCenter(0, k_up_center);

  glm::vec3 k_down_center(0);
	if (GetSegmentIntersection(ub, db, rb, lb, &psega, &psegb))
    k_down_center = ub + psega * (db - ub);
  cell.SetFaceCenter(1, k_down_center);

	glm::vec3 j_regular_center(0);
	if (GetSegmentIntersection(uf, ub, ul, ur, &psega, &psegb))
    j_regular_center = uf + psega * (ub - uf);
  cell.SetFaceCenter(5, j_regular_center);

	glm::vec3 j_inverted_center(0);
	if (GetSegmentIntersection(df, db, dl, dr, &psega, &psegb))
    j_inverted_center = df + psega * (db - df);
  cell.SetFaceCenter(4, j_inverted_center);

	glm::vec3 x_regular_center(0);
	if (GetSegmentIntersection(rf, rb, ur, dr, &psega, &psegb))
    x_regular_center = rf + psega * (rb - rf);
  cell.SetFaceCenter(2, x_regular_center);

  glm::vec3 x_inverted_center(0);
	if (GetSegmentIntersection(lf, lb, ul, dl, &psega, &psegb))
    x_inverted_center = lf + psega * (lb - lf);
  cell.SetFaceCenter(3, x_inverted_center);

	// Vertical-Depth Intersection
	glm::vec3 center(-FLT_MAX);
  if (GetSegmentIntersection(k_down_center, k_up_center, j_inverted_center, j_regular_center, &psega, &psegb))
    center = k_down_center + psega * (k_up_center - k_down_center);
  cell.SetCenter(center);
}

glm::mat3 Tank::GetCellJacobianInverse(const Cell& cell)
{
	//return glm::mat3(1, 0, 0, 0, 1, 0, 0, 0, 1);
	
	glm::vec3 x = cell.GetFaceCenter(2) - cell.GetFaceCenter(3);
	glm::vec3 y = cell.GetFaceCenter(5) - cell.GetFaceCenter(4);
	glm::vec3 z = cell.GetFaceCenter(0) - cell.GetFaceCenter(1);

  x *= m_scale;
  y *= m_scale;
  z *= m_scale;

  glm::mat3 jacob = glm::transpose(glm::mat3(x, y, z));
  return glm::inverse(jacob);
}

double Tank::CalculateGradient(const UINT32& x, const UINT32& y, const UINT32& z)
{
	double g = 0.0f;
	double dfx = 0.0f;
	double dfy = 0.0f;
	double dfz = 0.0f;

	double pdx = 0.0f;
	double pdy = 0.0f;
	double pdz = 0.0f;

	int h = MASK_SIZE / 2;
	int xinit = x - h;
	int yinit = y - h;
	int zinit = z - h;
	for ( int i = xinit; i < xinit + MASK_SIZE; ++i )
	{
		for ( int j = yinit; j < yinit + MASK_SIZE; ++j )
		{
			for ( int k = zinit; k < zinit + MASK_SIZE; ++k )
			{
        double dx = m_derivativeMask.GetDxAt(i - xinit, j - yinit, k - zinit);
        double dy = m_derivativeMask.GetDyAt(i - xinit, j - yinit, k - zinit);
        double dz = m_derivativeMask.GetDzAt(i - xinit, j - yinit, k - zinit);

				double v = GetValue(i, j, k);
				if ( IsOutOfBoundary(i, j, k) || !m_cells[GetId(i, j, k)].IsActive() )
				{
					v = GetValue(x, y, z);
				}
				pdx += abs(dx);
				pdy += abs(dy);
				pdz += abs(dz);

				dfx += dx * v;
				dfy += dy * v;
				dfz += dz * v;
		}
	}
}

	int id = GetId(x, y, z);
  m_grad[id].x = dfx / pdx;
	m_grad[id].y = dfy / pdy;
	m_grad[id].z = dfz / pdz;

	
	glm::mat3 jacob_inv = GetCellJacobianInverse(m_cells[id]);
  glm::vec3 grad = jacob_inv * m_grad[id];

	g = glm::length(grad);
	m_max_gradient = fmax(m_max_gradient, g);
	m_min_gradient = fmin(m_min_gradient, g);

	return g;
}

double Tank::CalculateLaplacian(const UINT32& x, const UINT32& y, const UINT32& z)
{
#ifndef HESSIAN
	double g = 0.0f;
	double dfx = 0.0f;
	double dfy = 0.0f;
	double dfz = 0.0f;

	double pdx = 0.0f;
	double pdy = 0.0f;
	double pdz = 0.0f;

	int h = MASK_SIZE / 2;
	int xinit = x - h;
	int yinit = y - h;
	int zinit = z - h;
	for (int i = xinit; i < xinit + MASK_SIZE; ++i) {
		for (int j = yinit; j < yinit + MASK_SIZE; ++j) {
			for (int k = zinit; k < zinit + MASK_SIZE; ++k) {
				double dx;
				double dy;
				double dz;
				m_derivativeMask.GetGradient(i - xinit, j - yinit, k - zinit, &dx, &dy, &dz);

				int gid = GetId(i, j, k);
				if (IsOutOfBoundary(i, j, k) || !m_cells[GetId(i, j, k)].IsActive()) {
					gid = GetId(x, y, z);
				}

        double gv = glm::length(GetCellJacobianInverse(m_cells[gid]) * m_grad[gid]);

				pdx += abs(dx);
				pdy += abs(dy);
				pdz += abs(dz);

				dfx += dx * gv;
				dfy += dy * gv;
				dfz += dz * gv;
			}
		}
	}

	dfx /= pdx;
	dfy /= pdy;
	dfz /= pdz;

	int id = GetId(x, y, z);

	glm::mat3 jacob_inv = GetCellJacobianInverse(m_cells[id]);
	glm::vec3 parametric_gradgrad(dfx, dfy, dfz);
  glm::vec3 gradgrad = jacob_inv * parametric_gradgrad;
	
  g = glm::dot(gradgrad, m_grad[id]) / glm::length(m_grad[id]);
	m_max_laplacian = fmax(m_max_laplacian, g);
	m_min_laplacian = fmin(m_min_laplacian, g);

	return g;
#else
	double fdxdx = 0.0f;
	double fdxdy = 0.0f;
	double fdxdz = 0.0f;

	double fdydx = 0.0f;
	double fdydy = 0.0f;
	double fdydz = 0.0f;

	double fdzdx = 0.0f;
	double fdzdy = 0.0f;
	double fdzdz = 0.0f;

	double pdx = 0.0f;
	double pdy = 0.0f;
	double pdz = 0.0f;

	int h = MASK_SIZE / 2;
	int xinit = x - h;
	int yinit = y - h;
	int zinit = z - h;
	for ( int i = xinit; i < xinit + MASK_SIZE; ++i )
	{
		for ( int j = yinit; j < yinit + MASK_SIZE; ++j )
		{
			for ( int k = zinit; k < zinit + MASK_SIZE; ++k )
			{
        double dx = m_derivativeMask.GetDxAt(i - xinit, j - yinit, k - zinit);
        double dy = m_derivativeMask.GetDyAt(i - xinit, j - yinit, k - zinit);
        double dz = m_derivativeMask.GetDzAt(i - xinit, j - yinit, k - zinit);

				int id = GetId(i, j, k);
				if ( IsOutOfBoundary(i, j, k) || !m_cells[GetId(i, j, k)].IsActive() )
					id = GetId(x, y, z);
				
        double fdx = m_grad[id].x;
				double fdy = m_grad[id].y;
				double fdz = m_grad[id].z;

				pdx += abs(dx);
				pdy += abs(dy);
				pdz += abs(dz);

				fdxdx += fdx * dx;
				fdxdy += fdx * dy;
				fdxdz += fdx * dz;

				fdydx += fdy * dx;
				fdydy += fdy * dy;
				fdydz += fdy * dz;

				fdzdx += fdz * dx;
				fdzdy += fdz * dy;
				fdzdz += fdz * dz;
		}
	}
}

	fdxdx /= pdx;
	fdxdy /= pdy;
	fdxdz /= pdz;
              
	fdydx /= pdx;
	fdydy /= pdy;
	fdydz /= pdz;
              
	fdzdx /= pdx;
	fdzdy /= pdy;
	fdzdz /= pdz;

	int id = GetId(x, y, z);
  double dfx = m_grad[id].x;
  double dfy = m_grad[id].y;
  double dfz = m_grad[id].z;

	glm::vec3 parametric_dx_grad(fdxdx, fdxdy, fdxdz);
	glm::vec3 parametric_dy_grad(fdydx, fdydy, fdydz);
	glm::vec3 parametric_dz_grad(fdzdx, fdzdy, fdzdz);

	glm::mat3 jacob_inv = GetCellJacobianInverse(m_cells[id]);
	glm::vec3 dx_grad = jacob_inv * parametric_dx_grad;
	glm::vec3 dy_grad = jacob_inv * parametric_dy_grad;
	glm::vec3 dz_grad = jacob_inv * parametric_dz_grad;

	glm::mat3 hess(dx_grad, dy_grad, dz_grad);
	glm::vec3 grad(dfx, dfy, dfz);

	double length = glm::length(grad);
	double sec_deriv = glm::dot((grad * hess), grad) / (length * length);

	m_min_laplacian = fmin(m_min_laplacian, sec_deriv);
	m_max_laplacian = fmax(m_max_laplacian, sec_deriv);
	return sec_deriv;
#endif
}

glm::dvec3* Tank::GetFaceVertices(const int& x, const int& y, const int& z, const int& face) const
{
  return GetFaceVertices(m_cells[GetId(x, y, z)], face);
}

glm::dvec3* Tank::GetFaceVertices(const Cell& cell, const int& face) const
{
  int* vertices_idx = cell.GetFaceVertices(face);
  if (vertices_idx)
  {
    glm::dvec3* vertices = new glm::dvec3[4];
    if (vertices)
    {
      vertices[0] = m_vertices[cell.GetIthVertexIndex(vertices_idx[0])];
      vertices[1] = m_vertices[cell.GetIthVertexIndex(vertices_idx[1])];
      vertices[2] = m_vertices[cell.GetIthVertexIndex(vertices_idx[2])];
      vertices[3] = m_vertices[cell.GetIthVertexIndex(vertices_idx[3])];
      delete[] vertices_idx;
      return vertices;
    }
  }
  return NULL;
}

bool Tank::IsParallelPlanes(const glm::dvec3& p0a, const glm::dvec3& p1a, const glm::dvec3& p0b, const glm::dvec3& p1b)
{
  glm::dvec3 pa_normal = glm::normalize(glm::cross(p0a, p1a));
  glm::dvec3 pb_normal = glm::normalize(glm::cross(p0b, p1b));
  glm::dvec3 diff = pa_normal + pb_normal;
  if (glm::bvec3(true) == glm::lessThanEqual(diff, glm::dvec3(0.0001f)))
    return true;
  return false;
}

bool Tank::IsFaceToFaceCells(const int& x, const int& y, const int& z, const int& i, const int& j, const int& k)
{
  int face = -1;
  int opposite = -1;
  int diffx = i - x;
  int diffy = j - y;
  int diffz = k - z;
  if (diffx != 0 ^ diffy != 0 ^ diffz != 0)
  {
    if (diffx > 0)
    {
      face = 3;
      opposite = 2;
    }
    else if (diffx < 0)
    {
      face = 2;
      opposite = 3;
    }
    else if (diffy > 0)
    {
      face = 5;
      opposite = 4;
    }
    else if (diffy < 0)
    {
      face = 4;
      opposite = 5;
    }
    else if (diffz > 0)
    {
      face = 1;
      opposite = 0;
    }
    else if (diffz < 0)
    {
      face = 0;
      opposite = 1;
    }
  }

  if (face == -1)
    return false;

  glm::dvec3* cell_verts = GetFaceVertices(x, y, z, face);
  glm::dvec3* opposite_verts = GetFaceVertices(i, j, k, opposite);
  if (!cell_verts || !opposite_verts)
    return false;

  bool is_parallel = IsParallelPlanes(cell_verts[0] - cell_verts[1], cell_verts[2] - cell_verts[1],
    opposite_verts[0] - opposite_verts[1], opposite_verts[2] - opposite_verts[1]);

  delete[] cell_verts;
  delete[] opposite_verts;

  return is_parallel;
}

void Tank::CalculateDerivatives(const UINT32& x, const UINT32& y, const UINT32& z, double* g, double* l)
{
	double fdx = 0.0f;
	double fdy = 0.0f;
	double fdz = 0.0f;

	double pdx = 0.0f;
	double pdy = 0.0f;
	double pdz = 0.0f;

	double fdxdx = 0.0f;
	double fdxdy = 0.0f;
	double fdxdz = 0.0f;
	double fdydy = 0.0f;
	double fdydz = 0.0f;
	double fdzdz = 0.0f;

	double pdxdx = 0.0f;
	double pdxdy = 0.0f;
	double pdxdz = 0.0f;
	double pdydy = 0.0f;
	double pdydz = 0.0f;
	double pdzdz = 0.0f;

	int h = MASK_SIZE / 2;
	int xinit = x - h;
	int yinit = y - h;
	int zinit = z - h;
	for ( int i = xinit; i < xinit + MASK_SIZE; ++i )
	{
		for ( int j = yinit; j < yinit + MASK_SIZE; ++j )
		{
			for ( int k = zinit; k < zinit + MASK_SIZE; ++k )
			{
				double dx = m_derivativeMask.GetDxAt(i - xinit, j - yinit, k - zinit);
				double dy = m_derivativeMask.GetDyAt(i - xinit, j - yinit, k - zinit);
				double dz = m_derivativeMask.GetDzAt(i - xinit, j - yinit, k - zinit);

				double dxdx = m_derivativeMask.GetDxdxAt(i - xinit, j - yinit, k - zinit);
				double dxdy = m_derivativeMask.GetDxdyAt(i - xinit, j - yinit, k - zinit);
				double dxdz = m_derivativeMask.GetDxdzAt(i - xinit, j - yinit, k - zinit);
				double dydy = m_derivativeMask.GetDydyAt(i - xinit, j - yinit, k - zinit);
				double dydz = m_derivativeMask.GetDydzAt(i - xinit, j - yinit, k - zinit);
				double dzdz = m_derivativeMask.GetDzdzAt(i - xinit, j - yinit, k - zinit);

				double v = GetValue(i, j, k);
				if (IsOutOfBoundary(i, j, k) || !m_cells[GetId(i, j, k)].IsActive() || !IsFaceToFaceCells(x, y, z, i, j, k))
				{
					v = GetValue(x, y, z);
				}
				
				fdxdx += dxdx * v;
				fdxdy += dxdy * v;
				fdxdz += dxdz * v;

				fdydy += dydy * v;
				fdydz += dydz * v;

				fdzdz += dzdz * v;

				pdxdx += abs(dxdx);
				pdxdy += abs(dxdy);
				pdxdz += abs(dxdz);
				pdydy += abs(dydy);
				pdydz += abs(dydz);
				pdzdz += abs(dzdz);

				fdx += dx * v;
				fdy += dy * v;
				fdz += dz * v;

				pdx += abs(dx);
				pdy += abs(dy);
				pdz += abs(dz);
			}
		}
	}

	int id = GetId(x, y, z);

  m_grad[id].x = fdx / pdx;
  m_grad[id].y = fdy / pdy;
  m_grad[id].z = fdz / pdz;
  
  fdxdx /= pdxdx;
  fdxdy /= pdxdy;
  fdxdz /= pdxdz;
  fdydy /= pdydy;
  fdydz /= pdydz;
  fdzdz /= pdzdz;
  m_hess[id][0][0] = fdxdx;
  m_hess[id][0][1] = fdxdy;
  m_hess[id][0][2] = fdxdz;
  m_hess[id][1][0] = fdxdy;
  m_hess[id][1][1] = fdydy;
  m_hess[id][1][2] = fdydz;
  m_hess[id][2][0] = fdxdz;
  m_hess[id][2][1] = fdydz;
  m_hess[id][2][2] = fdzdz;

  return UpdateDerivatives(x, y, z, g, l);
}

void Tank::UpdateDerivatives(const UINT32& x, const UINT32& y, const UINT32& z, double* g, double* l)
{
  //Returning gradient
  int id = GetId(x, y, z);
  glm::mat3 jacob_inv = GetCellJacobianInverse(m_cells[id]);
  glm::vec3 grad = jacob_inv * m_grad[id];
  double length = glm::length(grad);

  m_max_gradient = fmax(m_max_gradient, length);
  m_min_gradient = fmin(m_min_gradient, length);
  *g = length;

  if (length == 0.0f)
  {
    *l = -DBL_MAX;
    return;
  }

  double gg = 0.0f;
  double dfx = 0.0f;
  double dfy = 0.0f;
  double dfz = 0.0f;

  double pdx = 0.0f;
  double pdy = 0.0f;
  double pdz = 0.0f;

  int h = MASK_SIZE / 2;
  int xinit = x - h;
  int yinit = y - h;
  int zinit = z - h;
  for (int i = xinit; i < xinit + MASK_SIZE; ++i)
  {
    for (int j = yinit; j < yinit + MASK_SIZE; ++j)
    {
      for (int k = zinit; k < zinit + MASK_SIZE; ++k)
      {
        double dx;
        double dy;
        double dz;
        m_derivativeMask.GetGradient(i - xinit, j - yinit, k - zinit, &dx, &dy, &dz);

        int gid = GetId(i, j, k);
        if (IsOutOfBoundary(i, j, k) || !m_cells[GetId(i, j, k)].IsActive())
        {
          gid = GetId(x, y, z);
        }

        double gv = glm::length(GetCellJacobianInverse(m_cells[gid]) * m_grad[gid]);
        m_max_gradient = fmax(m_max_gradient, gv);
        m_min_gradient = fmin(m_min_gradient, gv);

        pdx += abs(dx);
        pdy += abs(dy);
        pdz += abs(dz);

        dfx += dx * gv;
        dfy += dy * gv;
        dfz += dz * gv;
      }
    }
  }

  dfx /= pdx;
  dfy /= pdy;
  dfz /= pdz;

  glm::vec3 parametric_gradgrad(dfx, dfy, dfz);
  glm::vec3 gradgrad = jacob_inv * parametric_gradgrad;

  *l = glm::dot(gradgrad, m_grad[id]) / glm::length(m_grad[id]);
  m_max_laplacian = fmax(m_max_laplacian, *l);
  m_min_laplacian = fmin(m_min_laplacian, *l);
}