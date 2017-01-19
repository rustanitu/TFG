#include "DerivativeMask.h"

#include <stdlib.h>

static const int GAUSSIAN_KERNEL[4][7] = {
	{ 1 },
	{ 1, 2, 1},
	{ 1, 4, 6, 4, 1 },
	{ 1, 6, 15, 20, 15, 6, 1 },
};

static const int FIRST_DERIVATIVE_KERNEL[4][7] = {
	{ 0 },
	{ -1, 0, 1 },
	{ 1, -8, 0, 8, -1 },
	{ -1, 9, -45, 0, 45, -9, 1 },
};

static const int FIRST_DERIVATIVE_KERNEL_D[4] = {1, 2, 12, 60};

static const int SECOND_DERIVATIVE_KERNEL[4][7] = {
	{ 0 },
	{ 1, -2, 1 },
	{ -1, 16, -30, 16, -1 },
	{ 2, -27, 270, -490, 270, -27, 2 },
};

static const int SECOND_DERIVATIVE_KERNEL_D[4] = { 1, 1, 12, 180 };

DerivativeMask::DerivativeMask(const int& n)
: m_n(n)
, m_gradient_mask(NULL)
, m_laplacian_mask(NULL)
{
	if (n % 2 == 0)
		m_n = n + 1;

	int i;
	int vsum;
	int half = m_n / 2;

	// Initializes gaussian kernel acording to the specified size
	m_h = new float[m_n];
	vsum = 0;
	for (i = 0; i < m_n; i++)
	{
		m_h[i] = GAUSSIAN_KERNEL[half][i];
		vsum += m_h[i];
	}

	//for (i = 0; i < m_n; i++) {
	//  m_h[i] = m_h[i] / vsum;
	//}

	// Initializes gradient kernel acording to the specified size
	m_hl = new float[m_n];
	for (i = 0; i < m_n; i++) {
		m_hl[i] = float(FIRST_DERIVATIVE_KERNEL[half][i]) / FIRST_DERIVATIVE_KERNEL_D[half];
	}

	// Initializes laplacian kernel acording to the specified size
	m_hll = new float[m_n];
	for (i = 0; i < m_n; i++) {
		m_hll[i] = float(SECOND_DERIVATIVE_KERNEL[half][i]) / SECOND_DERIVATIVE_KERNEL_D[half];
	}

	GenerateGradientMask();
	GenerateLaplacianMask();
	GenerateMasks();
}

unsigned int DerivativeMask::GetId(const int& x, const int& y, const int& z) const
{
	return x + (y * m_n) + (z * m_n * m_n);
}

void DerivativeMask::GenerateGradientMask()
{
	delete m_gradient_mask;
	m_gradient_mask = new float[m_n * m_n * m_n];

	for ( int x = 0; x < m_n; x++ )
	{
		float dk_val = m_hl[x];
		for ( int y = 0; y < m_n; y++ )
		{
			float dgy_val = m_h[y];
			for ( int z = 0; z < m_n; z++ )
			{
				float dgz_val = m_h[z];
				m_gradient_mask[GetId(x, y, z)] = dk_val * dgy_val * dgz_val;
			}
		}
	}
}

void DerivativeMask::GenerateLaplacianMask()
{
	delete m_laplacian_mask;
	m_laplacian_mask = new float[m_n * m_n * m_n];

	for ( int x = 0; x < m_n; x++ )
	{
		float dk_val = m_hll[x];
		for ( int y = 0; y < m_n; y++ )
		{
			float dgy_val = m_h[y];
			for ( int z = 0; z < m_n; z++ )
			{
				float dgz_val = m_h[z];
				m_laplacian_mask[GetId(x, y, z)] = dk_val * dgy_val * dgz_val;
			}
		}
	}
}

void DerivativeMask::GenerateMasks()
{
	const int size = m_n * m_n * m_n;
	m_dx_mask = new float[size];
	m_dy_mask = new float[size];
	m_dz_mask = new float[size];
	m_dxdx_mask = new float[size];
	m_dxdy_mask = new float[size];
	m_dxdz_mask = new float[size];
	m_dydy_mask = new float[size];
	m_dydz_mask = new float[size];
	m_dzdz_mask = new float[size];

	for (int x = 0; x < m_n; x++) {
		for (int y = 0; y < m_n; y++) {
			for (int z = 0; z < m_n; z++) {
				m_dx_mask[GetId(x, y, z)] = m_hl[x] * m_h[y] * m_h[z];
				m_dy_mask[GetId(x, y, z)] = m_h[x] * m_hl[y] * m_h[z];
				m_dz_mask[GetId(x, y, z)] = m_h[x] * m_h[y] * m_hl[z];
				m_dxdx_mask[GetId(x, y, z)] = m_hll[x] * m_h[y] * m_h[z];
				m_dxdy_mask[GetId(x, y, z)] = m_hl[x] * m_hl[y] * m_h[z];
				m_dxdz_mask[GetId(x, y, z)] = m_hl[x] * m_h[y] * m_hl[z];
				m_dydy_mask[GetId(x, y, z)] = m_h[x] * m_hll[y] * m_h[z];
				m_dydz_mask[GetId(x, y, z)] = m_h[x] * m_hl[y] * m_hl[z];
				m_dzdz_mask[GetId(x, y, z)] = m_h[x] * m_h[y] * m_hll[z];
			}
		}
	}
}

void DerivativeMask::GetGradient(const int& x, const int& y, const int& z, float* gx, float* gy, float* gz)
{
	if (x < 0 || y < 0 || z < 0 || x >= m_n || y >= m_n || z >= m_n) {
		gx = gy = gz = NULL;
		return;
	}

	*gx = m_gradient_mask[GetId(x, y, z)];
	*gy = m_gradient_mask[GetId(y, x, z)];
	*gz = m_gradient_mask[GetId(z, y, x)];
}

void DerivativeMask::GetLaplacian(const int& x, const int& y, const int& z, float* lx, float* ly, float* lz)
{
	if (x < 0 || y < 0 || z < 0 || x >= m_n || y >= m_n || z >= m_n) {
		lx = ly = lz = NULL;
		return;
	}

	*lx = m_laplacian_mask[GetId(x, y, z)];
	*ly = m_laplacian_mask[GetId(y, x, z)];
	*lz = m_laplacian_mask[GetId(z, y, x)];
}

float DerivativeMask::GetDxAt(const int& x, const int& y, const int& z) const
{
	if (x < 0 || y < 0 || z < 0 || x >= m_n || y >= m_n || z >= m_n)
		return 0.0f;
		
	return m_dx_mask[GetId(x, y, z)];
}

float DerivativeMask::GetDyAt(const int& x, const int& y, const int& z) const
{
	if (x < 0 || y < 0 || z < 0 || x >= m_n || y >= m_n || z >= m_n)
		return 0.0f;

	return m_dy_mask[GetId(x, y, z)];
}

float DerivativeMask::GetDzAt(const int& x, const int& y, const int& z) const
{
	if (x < 0 || y < 0 || z < 0 || x >= m_n || y >= m_n || z >= m_n)
		return 0.0f;

	return m_dz_mask[GetId(x, y, z)];
}

float DerivativeMask::GetDxdxAt(const int& x, const int& y, const int& z) const
{
	if (x < 0 || y < 0 || z < 0 || x >= m_n || y >= m_n || z >= m_n)
		return 0.0f;

	return m_dxdx_mask[GetId(x, y, z)];
}

float DerivativeMask::GetDxdyAt(const int& x, const int& y, const int& z) const
{
	if (x < 0 || y < 0 || z < 0 || x >= m_n || y >= m_n || z >= m_n)
		return 0.0f;

	return m_dxdy_mask[GetId(x, y, z)];
}

float DerivativeMask::GetDxdzAt(const int& x, const int& y, const int& z) const
{
	if (x < 0 || y < 0 || z < 0 || x >= m_n || y >= m_n || z >= m_n)
		return 0.0f;

	return m_dxdz_mask[GetId(x, y, z)];
}

float DerivativeMask::GetDydxAt(const int& x, const int& y, const int& z) const
{
	return GetDxdyAt(x, y, z);
}

float DerivativeMask::GetDydyAt(const int& x, const int& y, const int& z) const
{
	if (x < 0 || y < 0 || z < 0 || x >= m_n || y >= m_n || z >= m_n)
		return 0.0f;

	return m_dydy_mask[GetId(x, y, z)];
}

float DerivativeMask::GetDydzAt(const int& x, const int& y, const int& z) const
{
	if (x < 0 || y < 0 || z < 0 || x >= m_n || y >= m_n || z >= m_n)
		return 0.0f;

	return m_dydz_mask[GetId(x, y, z)];
}

float DerivativeMask::GetDzdxAt(const int& x, const int& y, const int& z) const
{
	return GetDxdzAt(x, y, z);
}

float DerivativeMask::GetDzdyAt(const int& x, const int& y, const int& z) const
{
	return GetDydzAt(x, y, z);
}

float DerivativeMask::GetDzdzAt(const int& x, const int& y, const int& z) const
{
	if (x < 0 || y < 0 || z < 0 || x >= m_n || y >= m_n || z >= m_n)
		return 0.0f;

	return m_dzdz_mask[GetId(x, y, z)];
}