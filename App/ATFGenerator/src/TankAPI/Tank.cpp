#include "Tank.h"
#include "Cell.h"

#include <algorithm>
#include <fstream>

#define MASK_SIZE 3

Tank::Tank()
: m_cells(NULL)
, m_vertices(NULL)
, m_current_timestep(0)
, m_derivativeMask(MASK_SIZE)
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
	m_scalar_fx = new float[m_ncells];
	m_scalar_fy = new float[m_ncells];
	m_scalar_fz = new float[m_ncells];
	if ( !m_cells || !m_scalar_fx || !m_scalar_fy || !m_scalar_fz )
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

		m_scalar_fx[id] = 0.0f;
		m_scalar_fy[id] = 0.0f;
		m_scalar_fz[id] = 0.0f;

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
			if ( active )
			{
				m_max_value = fmax(m_max_value, value);
				m_min_value = fmin(m_min_value, value);
			}
		}
	}

	return true;
}

float Tank::GetValue(const UINT32& x, const UINT32& y, const UINT32& z)
{
	if ( IsOutOfBoundary(x, y, z) )
		return m_min_value;

	return GetValue (GetId (x, y, z));
}

float Tank::GetValue(const UINT32& id)
{
	return m_cells[id].IsActive() ? m_cells[id].GetValue(m_current_timestep) : 0.0f;
}

float Tank::GetQuadraticGradientNorm(const UINT32& id)
{
	const float dfx = m_scalar_fx[id];
	const float dfy = m_scalar_fy[id];
	const float dfz = m_scalar_fz[id];
	return dfx*dfx + dfy*dfy + dfz*dfz;
}

float Tank::CalculateGradient(const UINT32& x, const UINT32& y, const UINT32& z)
{
	if ( !m_cells[GetId(x, y, z)].IsActive() )
		return 0.0f;

	float g = 0.0f;
	float dfx = 0.0f;
	float dfy = 0.0f;
	float dfz = 0.0f;

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
				dfx += dx * v;
				dfy += dy * v;
				dfz += dz * v;
			}
		}
	}

	int id = GetId(x, y, z);
	m_scalar_fx[id] = dfx;
	m_scalar_fy[id] = dfy;
	m_scalar_fz[id] = dfz;

	g = sqrt(GetQuadraticGradientNorm(id));
	m_max_gradient = fmax(m_max_gradient, g);

	return g;
}

float Tank::CalculateLaplacian(const UINT32& x, const UINT32& y, const UINT32& z)
{
	if (!m_cells[GetId(x, y, z)].IsActive())
		return 0.0f;

	float fdxdx = 0.0f;
	float fdxdy = 0.0f;
	float fdxdz = 0.0f;

	float fdydx = 0.0f;
	float fdydy = 0.0f;
	float fdydz = 0.0f;

	float fdzdx = 0.0f;
	float fdzdy = 0.0f;
	float fdzdz = 0.0f;

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

				int id = GetId(x, y, z);
				float dfx = 0.0f;//m_scalar_fx[id];
				float dfy = 0.0f;//m_scalar_fy[id];
				float dfz = 0.0f;//m_scalar_fz[id];

				if ( !IsOutOfBoundary(x, y, z) )
				{
					int id = GetId(i, j, k);
					dfx = m_scalar_fx[id];
					dfy = m_scalar_fy[id];
					dfz = m_scalar_fz[id];
				}

				fdxdx += dx * dfx;
				fdxdy += dx * dfy;
				fdxdz += dx * dfz;

				fdydx += dy * dfx;
				fdydy += dy * dfy;
				fdydz += dy * dfz;

				fdzdx += dz * dfx;
				fdzdy += dz * dfy;
				fdzdz += dz * dfz;
			}
		}
	}

	int id = GetId(x, y, z);
	float dfx = m_scalar_fx[id];
	float dfy = m_scalar_fy[id];
	float dfz = m_scalar_fz[id];
	float hess_x_gradient[3] = {fdxdx*dfx + fdydx*dfy + fdzdx*dfz, fdxdy*dfx + fdydy*dfy + fdzdy*dfz, fdxdz*dfx + fdydz*dfy + fdzdz*dfz};

	float sec_deriv = (hess_x_gradient[0] * dfx + hess_x_gradient[1] * dfy + hess_x_gradient[2] * dfz) / GetQuadraticGradientNorm(id);
	
	m_min_laplacian = fmin(m_min_laplacian, sec_deriv);
	m_max_laplacian = fmax(m_max_laplacian, sec_deriv);
	return sec_deriv;
}

//float CalculateGradientGradientByKernel(const UINT32& x, const UINT32& y, const UINT32& z)
//{
//	float dfx = 0.0f;
//	float dfy = 0.0f;
//	float dfz = 0.0f;
//
//	float dgx = 0.0f;
//	float dgy = 0.0f;
//	float dgz = 0.0f;
//
//	int h = MASK_SIZE / 2;
//	int xinit = x - h;
//	int yinit = y - h;
//	int zinit = z - h;
//	for ( int i = xinit; i < xinit + MASK_SIZE; ++i )
//	{
//		for ( int j = yinit; j < yinit + MASK_SIZE; ++j )
//		{
//			for ( int k = zinit; k < zinit + MASK_SIZE; ++k )
//			{
//				float dx, dy, dz;
//				m_derivativeMask.GetGradient(i - xinit, j - yinit, k - zinit, &dx, &dy, &dz);
//
//				float f = GetValue(i, j, k);
//				dfx += dx * f;
//				dfy += dy * f;
//				dfz += dz * f;
//
//				float g = m_scalar_gradient[GetId(i, j, k)];
//				dgx += dx * g;
//				dgy += dy * g;
//				dgz += dz * g;
//			}
//		}
//	}
//
//	float gm = m_scalar_gradient[GetId(x, y, z)];
//
//	float gg = (dgx * dfx + dgy * dfy + dgz * dfz) / fmax(gm, 0.000001f);
//	m_min_laplacian = fmin(m_min_laplacian, gg);
//	m_max_laplacian = fmax(m_max_laplacian, gg);
//
//	return gg;
//}

//float CalculateLaplacianByKernel(const UINT32& x, const UINT32& y, const UINT32& z)
//{
//	if ( !m_scalarfield )
//		throw std::exception_ptr();
//
//	float l = 0.0f;
//	float lx = 0.0f;
//	float ly = 0.0f;
//	float lz = 0.0f;
//
//	int h = MASK_SIZE / 2;
//	int xinit = x - h;
//	int yinit = y - h;
//	int zinit = z - h;
//	for ( int i = xinit; i < xinit + MASK_SIZE; ++i )
//	{
//		for ( int j = yinit; j < yinit + MASK_SIZE; ++j )
//		{
//			for ( int k = zinit; k < zinit + MASK_SIZE; ++k )
//			{
//				float dx;
//				float dy;
//				float dz;
//				m_derivativeMask.GetLaplacian(i - xinit, j - yinit, k - zinit, &dx, &dy, &dz);
//
//				float v = GetValue(i, j, k);
//				lx += dx * v;
//				ly += dy * v;
//				lz += dz * v;
//			}
//		}
//	}
//
//	l = lx + ly + lz;
//
//	m_scalarfield->m_max_laplacian = fmax(m_scalarfield->m_max_laplacian, l);
//	m_scalarfield->m_min_laplacian = fmin(m_scalarfield->m_min_laplacian, l);
//
//	return l;
//}