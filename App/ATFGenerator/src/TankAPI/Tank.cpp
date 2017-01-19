#include "Tank.h"

#include <algorithm>
#include <fstream>
#include <vector>

//#define HESSIAN

Tank::Tank()
	: m_cells(NULL)
	, m_vertices(NULL)
	, m_current_timestep(0)
	, m_scalar_fx(NULL)
	, m_scalar_fy(NULL)
	, m_scalar_fz(NULL)
{
	printf("Tank criado.\n");
}

Tank::~Tank()
{
	printf("Tank destruido.\n");
	m_steps.clear();

	delete[] m_cells;
	delete[] m_vertices;
	delete[] m_scalar_fx;
	delete[] m_scalar_fy;
	delete[] m_scalar_fz;
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
	m_scalar_fx = new float[m_ncells];
	m_scalar_fy = new float[m_ncells];
	m_scalar_fz = new float[m_ncells];
	if ( !m_cells || !m_scalar_fx || !m_scalar_fy || !m_scalar_fz )
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

	std::vector<int> inactives;

	// It gets all cells
	for ( int c = 0; c < m_ncells; ++c )
	{
		bool active;
		int i, j, k;
		if ( !(file >> active >> i >> j >> k) )
			return false;

		int id = GetId(i, j, k);
		Cell* cell = &(m_cells[id]);
		cell->Init(i, j, k, active, m_nsteps);

		m_scalar_fx[id] = 0.0f;
		m_scalar_fy[id] = 0.0f;
		m_scalar_fz[id] = 0.0f;

		// It sets the index of the ith vertex
		for ( int v = 0; v < 8; ++v )
		{
			int index;
			if ( !(file >> index) )
				return false;

			cell->SetIthVertexIndex(v, index);
		}

    int siblids[6] = {
      GetId(i + 1, j, k),
      GetId(i - 1, j, k),
      GetId(i, j + 1, k),
      GetId(i, j - 1, k),
      GetId(i, j, k + 1),
      GetId(i, j, k - 1),
    };

		// It sets the index of the adjacent ith cell
		for ( int a = 0; a < 6; ++a )
		{
			int index;
			if ( !(file >> index) )
				return false;

      if (index != -1) {
        bool match = false;
        for (int b = 0; b < 6; ++b) {
          if (index == siblids[b]) {
            match = true;
            break;
          }
        }
        if (!match)
        {
          printf("O mapeamento do volume difere do experado.\n");
          delete[] m_cells;
          delete[] m_scalar_fx;
          delete[] m_scalar_fy;
          delete[] m_scalar_fz;
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
			float value;
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
				inactives.push_back(id);
			}
		}
	}

	printf("Fim da leitura do tank.\n");

	return true;
}

bool Tank::ReadFromVolume(const UINT32& width, const UINT32& height, const UINT32& depth, float* values)
{
	m_nvertices = 0;
	m_vertices = NULL;
	m_nsteps = 1;

	m_width = width;
	m_height = height;
	m_depth = depth;

	printf("Tamanho do Tank:\n");
	printf(" - tWidth: %d\n", m_width);
	printf(" - tHeight: %d\n", m_height);
	printf(" - tDepth: %d\n", m_depth);

	m_ncells = m_width * m_height * m_depth;
	m_cells = new Cell[m_ncells];
	m_scalar_fx = new float[m_ncells];
	m_scalar_fy = new float[m_ncells];
	m_scalar_fz = new float[m_ncells];
	if ( !m_cells || !m_scalar_fx || !m_scalar_fy || !m_scalar_fz )
		return false;

	// It gets all cells
	for ( int i = 0; i < m_width; ++i )
	{
		for ( int j = 0; j < m_height; ++j )
		{
			for ( int k = 0; k < m_depth; ++k )
			{
				bool active = true;

				int id = GetId(i, j, k);
				Cell* cell = &(m_cells[id]);
				cell->Init(i, j, k, active, m_nsteps);

				m_scalar_fx[id] = 0.0f;
				m_scalar_fy[id] = 0.0f;
				m_scalar_fz[id] = 0.0f;

				cell->SetValue(0, values[id]);
				m_max_value = fmax(m_max_value, values[id]);
				m_min_value = fmin(m_min_value, values[id]);
			}
		}
	}

	printf("Fim da leitura do tank.\n");
}

float Tank::GetValue(const UINT32& x, const UINT32& y, const UINT32& z)
{
	if ( IsOutOfBoundary(x, y, z) )
		return -FLT_MAX;

	return GetValue(GetId(x, y, z));
}

float Tank::GetValue(const UINT32& id)
{
	return m_cells[id].GetValue(m_current_timestep);
  return m_cells[id].IsActive() ? m_cells[id].GetValue(m_current_timestep) : -FLT_MAX;
}

float Tank::GetQuadraticGradientNorm(const UINT32& id)
{
	const float dfx = m_scalar_fx[id];
	const float dfy = m_scalar_fy[id];
	const float dfz = m_scalar_fz[id];
	return dfx*dfx + dfy*dfy + dfz*dfz;
}

bool Tank::GetSegmentIntersection(const glm::vec3& k, const glm::vec3& l, const glm::vec3& m, const glm::vec3& n, float* s, float* t)
{
  double det;


  if (((k.x != l.x) || (k.y != l.y)) &&
    ((m.x != n.x) || (m.y != n.y)))  /* se nao e' paralela ao plano XY*/
  {
    det = (n.x - m.x) * (l.y - k.y) - (n.y - m.y) * (l.x - k.x);

    if (det == 0.0)
      return false;

    *s = ((n.x - m.x) * (m.y - k.y) - (n.y - m.y) * (m.x - k.x)) / det;
    *t = ((l.x - k.x) * (m.y - k.y) - (l.y - k.y) * (m.x - k.x)) / det;

    return true;
  }

  if (((k.x != l.x) || (k.z != l.z)) &&
    ((m.x != n.x) || (m.z != n.z)))  /* se nao e' paralela ao plano XZ*/
  {
    det = (n.x - m.x) * (l.z - k.z) - (n.z - m.z) * (l.x - k.x);

    if (det == 0.0)
      return false;

    *s = ((n.x - m.x) * (m.z - k.z) - (n.z - m.z) * (m.x - k.x)) / det;
    *t = ((l.x - k.x) * (m.z - k.z) - (l.z - k.z) * (m.x - k.x)) / det;

    return true;
  }

  if (((k.y != l.y) || (k.z != l.z)) &&
    ((m.y != n.y) || (m.z != n.z)))  /* se nao e' paralela ao plano YZ*/
  {
    det = (n.y - m.y) * (l.z - k.z) - (n.z - m.z) * (l.y - k.y);

    if (det == 0.0)
      return false;

    *s = ((n.y - m.y) * (m.z - k.z) - (n.z - m.z) * (m.y - k.y)) / det;
    *t = ((l.y - k.y) * (m.z - k.z) - (l.z - k.z) * (m.y - k.y)) / det;

    return true;
  }

  return true;
}

void Tank::FillCellAdjCenter(Cell& cell)
{
  //midle edge definition
  //
  //         6____ub____7
  //        /|         /
  //      ul |       ur|
  //      /  lb      / rb
  //    2/___|uf____/3 |
  //    |   4|___db|___|5
  //    |   /      |   /
  //    lf dl      rf dr
  //    | /        | /
  //    |/____df___|/
  //    0          1
  //
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

  // Front face center
  glm::vec3 ffc(0);
  if (GetSegmentIntersection(uf, df, rf, lf, &psega, &psegb))
    ffc = uf + psega * (df - uf);

  // Back face center
  glm::vec3 bfc(0);
  if (GetSegmentIntersection(ub, db, rb, lb, &psega, &psegb))
    bfc = ub + psega * (db - ub);

  // Up face center
  glm::vec3 ufc(0);
  if (GetSegmentIntersection(uf, ub, ul, ur, &psega, &psegb))
    ufc = uf + psega * (ub - uf);

  // Down face center
  glm::vec3 dfc(0);
  if (GetSegmentIntersection(df, db, dl, dr, &psega, &psegb))
    dfc = df + psega * (db - df);

  // Redudant Code
  /*
  // Right face center
  glm::vec3 rfc(0);
  if (GetSegmentIntersection(rf, rb, ur, dr, &psega, &psegb))
    rfc = rf + psega * (rb - rf);

  // Left face center
  glm::vec3 lfc(0);
  if (GetSegmentIntersection(lf, lb, ul, dl, &psega, &psegb))
    lfc = lf + psega * (lb - lf);

  // Vertical-Horizontal Intersection
  glm::vec3 vhi(0);
  if (GetSegmentIntersection(ufc, dfc, lfc, rfc, &psega, &psegb))
    vhi = ufc + psega * (dfc - ufc);

  // Depth-Horizontal Intersection
  glm::vec3 dhi(0);
  if (GetSegmentIntersection(ffc, bfc, lfc, rfc, &psega, &psegb))
    dhi = ffc + psega * (bfc - ffc);
   
   //*/

  // Vertical-Depth Intersection
  glm::vec3 vdi(-FLT_MAX);
  if (GetSegmentIntersection(ufc, dfc, ffc, bfc, &psega, &psegb))
    vdi = ufc + psega * (dfc - ufc);
  cell.SetCenter(vdi);

  // Right Face Intersection
  glm::vec3 rfi(-FLT_MAX);
  if (GetSegmentIntersection(rf, rb, dr, ur, &psega, &psegb))
    rfi = rf + psega * (rb - rf);
  cell.SetFaceCenter(2, rfi);

  // Left Face Intersection
  glm::vec3 lfi(-FLT_MAX);
  if (GetSegmentIntersection(lf, lb, dl, ul, &psega, &psegb))
    lfi = lf + psega * (lb - lf);
  cell.SetFaceCenter(3, lfi);

  // Front Face Intersection
  glm::vec3 ffi(-FLT_MAX);
  if (GetSegmentIntersection(uf, df, rf, lf, &psega, &psegb))
    ffi = uf + psega * (df - uf);
  cell.SetFaceCenter(0, ffi);

  // Back Face Intersection
  glm::vec3 bfi(-FLT_MAX);
  if (GetSegmentIntersection(ub, db, rb, lb, &psega, &psegb))
    bfi = ub + psega * (db - ub);
  cell.SetFaceCenter(1, bfi);

  // Up Face Intersection
  glm::vec3 ufi(-FLT_MAX);
  if (GetSegmentIntersection(uf, ub, ur, ul, &psega, &psegb))
    ufi = uf + psega * (ub - uf);
  cell.SetFaceCenter(5, ufi);

  // Up Face Intersection
  glm::vec3 dfi(-FLT_MAX);
  if (GetSegmentIntersection(df, db, dr, dl, &psega, &psegb))
    dfi = df + psega * (db - df);
  cell.SetFaceCenter(4, dfi);
}

glm::mat3 Tank::GetCellJacobianInverse(const Cell& cell)
{
  //return glm::mat3(1, 0, 0, 0, 1, 0, 0, 0, 1);
  int id = GetId(cell.GetI(), cell.GetJ(), cell.GetK());
  
  glm::vec3 x = cell.GetFaceCenter(3) - cell.GetFaceCenter(2);
  int idxp = cell.GetAdjcentCellIndex(3);
  int idxn = cell.GetAdjcentCellIndex(2);
  if (idxp != -1 && idxn != -1)
    x = (m_cells[idxp].GetCenter() - m_cells[idxn].GetCenter()) / 2.0f;

  glm::vec3 y = cell.GetFaceCenter(4) - cell.GetFaceCenter(5);
  int idyp = cell.GetAdjcentCellIndex(4);
  int idyn = cell.GetAdjcentCellIndex(5);
  if (idyp != -1 && idyn != -1)
    y = (m_cells[idyp].GetCenter() - m_cells[idyn].GetCenter()) / 2.0f;

  glm::vec3 z = cell.GetFaceCenter(0) - cell.GetFaceCenter(1);
  int idzp = cell.GetAdjcentCellIndex(0);
  int idzn = cell.GetAdjcentCellIndex(1);
  if (idzp != -1 && idzn != -1)
    z = (m_cells[idzp].GetCenter() - m_cells[idzn].GetCenter()) / 2.0f;

  glm::mat3 jacob = glm::inverse(glm::transpose(glm::mat3(glm::normalize(x), glm::normalize(y), glm::normalize(z))));
  return jacob;
}

float Tank::CalculateGradient(const UINT32& x, const UINT32& y, const UINT32& z)
{
	float g = 0.0f;
	float dfx = 0.0f;
	float dfy = 0.0f;
	float dfz = 0.0f;

	float pdx = 0.0f;
	float pdy = 0.0f;
	float pdz = 0.0f;

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
				float dx;
				float dy;
				float dz;
				m_derivativeMask.GetGradient(i - xinit, j - yinit, k - zinit, &dx, &dy, &dz);

				float v = GetValue(i, j, k);
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

	dfx /= pdx;
	dfy /= pdy;
	dfz /= pdz;

	int id = GetId(x, y, z);
	
  glm::mat3 jacob_inv = GetCellJacobianInverse(m_cells[id]);
  glm::vec3 parametric_grad(dfx, dfy, dfz);
  glm::vec3 grad = jacob_inv * parametric_grad;

  m_scalar_fx[id] = grad.x;
  m_scalar_fy[id] = grad.y;
  m_scalar_fz[id] = grad.z;

	g = glm::length(grad);
	m_max_gradient = fmax(m_max_gradient, g);

	return g;
}

float Tank::CalculateLaplacian(const UINT32& x, const UINT32& y, const UINT32& z)
{
#ifndef HESSIAN
  float g = 0.0f;
  float dfx = 0.0f;
  float dfy = 0.0f;
  float dfz = 0.0f;

  float pdx = 0.0f;
  float pdy = 0.0f;
  float pdz = 0.0f;

  int h = MASK_SIZE / 2;
  int xinit = x - h;
  int yinit = y - h;
  int zinit = z - h;
  for (int i = xinit; i < xinit + MASK_SIZE; ++i) {
    for (int j = yinit; j < yinit + MASK_SIZE; ++j) {
      for (int k = zinit; k < zinit + MASK_SIZE; ++k) {
        float dx;
        float dy;
        float dz;
        m_derivativeMask.GetGradient(i - xinit, j - yinit, k - zinit, &dx, &dy, &dz);

        int gid = GetId(i, j, k);
        if (IsOutOfBoundary(i, j, k) || !m_cells[GetId(i, j, k)].IsActive()) {
          gid = GetId(x, y, z);
        }

        float dgx = m_scalar_fx[gid];
        float dgy = m_scalar_fy[gid];
        float dgz = m_scalar_fz[gid];
        glm::vec3 ggrad(dgx, dgy, dgz);
        float gv = glm::length(ggrad);

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
  glm::vec3 parametric_grad(dfx, dfy, dfz);
  glm::vec3 grad = jacob_inv * parametric_grad;
  glm::vec3 fgrad(m_scalar_fx[id], m_scalar_fy[id], m_scalar_fz[id]);
  
  g = glm::dot(grad, fgrad) / glm::length(fgrad);
  m_max_laplacian = fmax(m_max_laplacian, g);
  m_min_laplacian = fmin(m_min_laplacian, g);

  return g;
#else
	float fdxdx = 0.0f;
	float fdxdy = 0.0f;
	float fdxdz = 0.0f;

	float fdydx = 0.0f;
	float fdydy = 0.0f;
	float fdydz = 0.0f;

	float fdzdx = 0.0f;
	float fdzdy = 0.0f;
	float fdzdz = 0.0f;

	float pdx = 0.0f;
	float pdy = 0.0f;
	float pdz = 0.0f;

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
				float dx, dy, dz;
				m_derivativeMask.GetGradient(i - xinit, j - yinit, k - zinit, &dx, &dy, &dz);

				int id = GetId(i, j, k);
				float fdx = m_scalar_fx[id];
				float fdy = m_scalar_fy[id];
				float fdz = m_scalar_fz[id];

				if ( IsOutOfBoundary(i, j, k) || !m_cells[GetId(i, j, k)].IsActive() )
				{
					id = GetId(x, y, z);

          fdx = m_scalar_fx[id];
          fdy = m_scalar_fy[id];
          fdz = m_scalar_fz[id];
				}

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
	float dfx = m_scalar_fx[id];
	float dfy = m_scalar_fy[id];
	float dfz = m_scalar_fz[id];

  glm::vec3 parametric_dx_grad(fdxdx, fdxdy, fdxdz);
  glm::vec3 parametric_dy_grad(fdydx, fdydy, fdydz);
  glm::vec3 parametric_dz_grad(fdzdx, fdzdy, fdzdz);

  glm::mat3 jacob_inv = GetCellJacobianInverse(m_cells[id]);
  glm::vec3 dx_grad = jacob_inv * parametric_dx_grad;
  glm::vec3 dy_grad = jacob_inv * parametric_dy_grad;
  glm::vec3 dz_grad = jacob_inv * parametric_dz_grad;

  glm::mat3 hess(dx_grad, dy_grad, dz_grad);

  glm::vec3 grad(dfx, dfy, dfz);

  float length = glm::length(grad);
  float sec_deriv = glm::dot(grad, (grad * hess)) / (length * length);

	m_min_laplacian = fmin(m_min_laplacian, sec_deriv);
	m_max_laplacian = fmax(m_max_laplacian, sec_deriv);
	return sec_deriv;
#endif
}

void Tank::CalculateDerivatives(const UINT32& x, const UINT32& y, const UINT32& z, float* g, float* l)
{
  float fdx = 0.0f;
  float fdy = 0.0f;
  float fdz = 0.0f;

  float pdx = 0.0f;
  float pdy = 0.0f;
  float pdz = 0.0f;

	float fdxdx = 0.0f;
	float fdxdy = 0.0f;
	float fdxdz = 0.0f;
	float fdydy = 0.0f;
	float fdydz = 0.0f;
	float fdzdz = 0.0f;

  float pdxdx = 0.0f;
  float pdxdy = 0.0f;
  float pdxdz = 0.0f;
  float pdydy = 0.0f;
  float pdydz = 0.0f;
  float pdzdz = 0.0f;

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
        float dx = m_derivativeMask.GetDxAt(i - xinit, j - yinit, k - zinit);
        float dy = m_derivativeMask.GetDyAt(i - xinit, j - yinit, k - zinit);
        float dz = m_derivativeMask.GetDzAt(i - xinit, j - yinit, k - zinit);

        float dxdx = m_derivativeMask.GetDxdxAt(i - xinit, j - yinit, k - zinit);
        float dxdy = m_derivativeMask.GetDxdyAt(i - xinit, j - yinit, k - zinit);
        float dxdz = m_derivativeMask.GetDxdzAt(i - xinit, j - yinit, k - zinit);
        float dydy = m_derivativeMask.GetDydyAt(i - xinit, j - yinit, k - zinit);
        float dydz = m_derivativeMask.GetDydzAt(i - xinit, j - yinit, k - zinit);
        float dzdz = m_derivativeMask.GetDzdzAt(i - xinit, j - yinit, k - zinit);

        float v = GetValue(i, j, k);
				if (IsOutOfBoundary(i, j, k) || !m_cells[GetId(i, j, k)].IsActive())
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

	//Returning gradient
	int id = GetId(x, y, z);
	glm::mat3 jacob_inv = GetCellJacobianInverse(m_cells[id]);

	glm::vec3 parametric_grad(fdx / pdx, fdy / pdy, fdz / pdz);
	glm::vec3 grad = jacob_inv * parametric_grad;
	*g = glm::length(grad);
	m_max_gradient = fmax(m_max_gradient, *g);

	//Returning laplacian
	fdxdx /= pdxdx;
	fdxdy /= pdxdy;
	fdxdz /= pdxdz;
	fdydy /= pdydy;
	fdydz /= pdydz;
	fdzdz /= pdzdz;
  
  glm::vec3 parametric_dx_grad(fdxdx, fdxdy, fdxdz);
  glm::vec3 parametric_dy_grad(fdxdy, fdydy, fdydz);
  glm::vec3 parametric_dz_grad(fdxdz, fdydz, fdzdz);

  glm::vec3 dx_grad = jacob_inv * parametric_dx_grad;
  glm::vec3 dy_grad = jacob_inv * parametric_dy_grad;
  glm::vec3 dz_grad = jacob_inv * parametric_dz_grad;

  glm::mat3 hess(dx_grad, dy_grad, dz_grad);

  float length = glm::length(grad);
  float sec_deriv = glm::dot(grad, (grad * hess)) / (length * length);

	m_min_laplacian = fmin(m_min_laplacian, sec_deriv);
	m_max_laplacian = fmax(m_max_laplacian, sec_deriv);

  *l = sec_deriv;
}