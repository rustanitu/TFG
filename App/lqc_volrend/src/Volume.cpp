#include <volrend/Volume.h>

#include <lqc/File/RAWLoader.h>
#include <math/MUtils.h>
#include <math/MSVD.h>
#include <math/MGeometry.h>

#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>

#define HESSIAN

namespace vr
{

	Volume::Volume() : ScalarField()
		, m_scalar_values(NULL)
		, m_scalar_fx(NULL)
		, m_scalar_fy(NULL)
		, m_scalar_fz(NULL)
	{
	}

	Volume::Volume(const UINT32& width, const UINT32& height, const UINT32& depth)
		: ScalarField(width, height, depth)
	{
		int size = m_width*m_height*m_depth;
		m_scalar_values = new double[size];
		//m_scalar_fx = new double[size];
		//m_scalar_fy = new double[size];
		//m_scalar_fz = new double[size];
		for ( int i = 0; i < size; i++ )
		{
			m_scalar_values[i] = 0.0f;
			//m_scalar_fx[i] = 0.0f;
			//m_scalar_fy[i] = 0.0f;
			//m_scalar_fz[i] = 0.0f;
		}
	}

	Volume::Volume(const UINT32& width, const UINT32& height, const UINT32& depth, double* scalars)
		: ScalarField(width, height, depth)
	{
		int size = m_width*m_height*m_depth;
		m_scalar_values = new double[size];
		//m_scalar_fx = new double[size];
		//m_scalar_fy = new double[size];
		//m_scalar_fz = new double[size];
		for ( int i = 0; i < size; i++ )
		{
			double v = scalars[i];
			m_scalar_values[i] = v;
			m_max_value = fmax(m_max_value, v);
			m_min_value = fmin(m_min_value, v);
			//m_scalar_fx[i] = 0.0f;
			//m_scalar_fy[i] = 0.0f;
			//m_scalar_fz[i] = 0.0f;
		}
	}

	Volume::Volume(const UINT32& width, const UINT32& height, const UINT32& depth, unsigned char* scalars)
		: ScalarField(width, height, depth)
	{
		int size = m_width * m_height * m_depth;
		m_scalar_values = new double[size];
		//m_scalar_fx = new double[size];
		//m_scalar_fy = new double[size];
		//m_scalar_fz = new double[size];
		for (int i = 0; i < size; i++)
		{
			double v = scalars[i];
			m_scalar_values[i] = v;
			m_max_value = fmax(m_max_value, v);
			m_min_value = fmin(m_min_value, v);
			//m_scalar_fx[i] = 0.0f;
			//m_scalar_fy[i] = 0.0f;
			//m_scalar_fz[i] = 0.0f;
		}
	}

	Volume::~Volume()
	{
		printf("\n--Volume destruído--\n");
		delete[] m_scalar_values;
	}

	lqc::Vector3f Volume::GetAnchorMin()
	{
		return m_pmin;
	}

	lqc::Vector3f Volume::GetAnchorMax()
	{
		return m_pmax;
	}

	void Volume::SetAnchors(lqc::Vector3f pmin, lqc::Vector3f pmax)
	{
		m_pmin = pmin;
		m_pmax = pmax;
	}

	double Volume::GetValue(const UINT32& x, const UINT32& y, const UINT32& z)
	{
		UINT32 xt = std::min(x, m_width - 1);
		UINT32 yt = std::min(y, m_height - 1);
		UINT32 zt = std::min(z, m_depth - 1);

		return m_scalar_values[GetId(xt, yt, zt)];
	}

	double Volume::CalculateGradient(const UINT32& x, const UINT32& y, const UINT32& z)
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
		for (int i = xinit; i < xinit + MASK_SIZE; ++i) {
			for (int j = yinit; j < yinit + MASK_SIZE; ++j) {
				for (int k = zinit; k < zinit + MASK_SIZE; ++k) {
					double dx;
					double dy;
					double dz;
					m_derivativeMask.GetGradient(i - xinit, j - yinit, k - zinit, &dx, &dy, &dz);

					double v = GetValue(i, j, k);
					if (IsOutOfBoundary(i, j, k)) {
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

		glm::vec3 grad(dfx, dfy, dfz);

		//m_scalar_fx[id] = grad.x;
		//m_scalar_fy[id] = grad.y;
		//m_scalar_fz[id] = grad.z;

		g = glm::length(grad);
		m_max_gradient = fmax(m_max_gradient, g);
		m_min_gradient = fmin(m_min_gradient, g);

		return g;
	}

	double Volume::CalculateLaplacian(const UINT32& x, const UINT32& y, const UINT32& z)
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
					if (IsOutOfBoundary(i, j, k)) {
						gid = GetId(x, y, z);
					}

					double dgx = m_scalar_fx[gid];
					double dgy = m_scalar_fy[gid];
					double dgz = m_scalar_fz[gid];
					glm::vec3 ggrad(dgx, dgy, dgz);
					double gv = glm::length(ggrad);

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

		glm::vec3 grad(dfx, dfy, dfz);
		glm::vec3 fgrad(m_scalar_fx[id], m_scalar_fy[id], m_scalar_fz[id]);

		g = glm::dot(grad, fgrad) / glm::length(fgrad);
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
		for (int i = xinit; i < xinit + MASK_SIZE; ++i) {
			for (int j = yinit; j < yinit + MASK_SIZE; ++j) {
				for (int k = zinit; k < zinit + MASK_SIZE; ++k) {
					double dx, dy, dz;
					m_derivativeMask.GetGradient(i - xinit, j - yinit, k - zinit, &dx, &dy, &dz);

					int id = GetId(i, j, k);
					double fdx = m_scalar_fx[id];
					double fdy = m_scalar_fy[id];
					double fdz = m_scalar_fz[id];

					if (IsOutOfBoundary(i, j, k)) {
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
		double dfx = m_scalar_fx[id];
		double dfy = m_scalar_fy[id];
		double dfz = m_scalar_fz[id];

		glm::vec3 dx_grad(fdxdx, fdxdy, fdxdz);
		glm::vec3 dy_grad(fdydx, fdydy, fdydz);
		glm::vec3 dz_grad(fdzdx, fdzdy, fdzdz);

		glm::mat3 hess(dx_grad, dy_grad, dz_grad);

		glm::vec3 grad(dfx, dfy, dfz);

		double length = glm::length(grad);
		double sec_deriv = glm::dot(grad, (grad * hess)) / (length * length);

		m_min_laplacian = fmin(m_min_laplacian, sec_deriv);
		m_max_laplacian = fmax(m_max_laplacian, sec_deriv);
		return sec_deriv;
#endif
	}

	void Volume::CalculateDerivatives(const UINT32& x, const UINT32& y, const UINT32& z, double* g, double* l)
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
		for (int i = xinit; i < xinit + MASK_SIZE; ++i) {
			for (int j = yinit; j < yinit + MASK_SIZE; ++j) {
				for (int k = zinit; k < zinit + MASK_SIZE; ++k) {
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
					if (IsOutOfBoundary(i, j, k)) {
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
		glm::vec3 grad(fdx / pdx, fdy / pdy, fdz / pdz);
		*g = glm::length(grad);
		m_max_gradient = fmax(m_max_gradient, *g);
		m_min_gradient = fmin(m_min_gradient, *g);

		//Returning laplacian
		fdxdx /= pdxdx;
		fdxdy /= pdxdy;
		fdxdz /= pdxdz;
		fdydy /= pdydy;
		fdydz /= pdydz;
		fdzdz /= pdzdz;

		glm::vec3 dx_grad(fdxdx, fdxdy, fdxdz);
		glm::vec3 dy_grad(fdxdy, fdydy, fdydz);
		glm::vec3 dz_grad(fdxdz, fdydz, fdzdz);

		glm::mat3 hess(dx_grad, dy_grad, dz_grad);

		double length = glm::length(grad);
		double sec_deriv = glm::dot(grad, (grad * hess)) / (length * length);

		m_min_laplacian = fmin(m_min_laplacian, sec_deriv);
		m_max_laplacian = fmax(m_max_laplacian, sec_deriv);

		*l = sec_deriv;
	}

	double Volume::InterpolatedValue(double px, double py, double pz)
	{
		double x = ((px - m_pmin.x) / (m_pmax.x - m_pmin.x)) * (double) (m_width - 1);
		double y = ((py - m_pmin.y) / (m_pmax.y - m_pmin.y)) * (double) (m_height - 1);
		double z = ((pz - m_pmin.z) / (m_pmax.z - m_pmin.z)) * (double) (m_depth - 1);

		UINT32 x0 = (int) x; UINT32 x1 = x0 + 1;
		UINT32 y0 = (int) y; UINT32 y1 = y0 + 1;
		UINT32 z0 = (int) z; UINT32 z1 = z0 + 1;

		if ( x0 == (double) (m_width - 1) )
		{
			x1 = (int) x0;
			x0 = x1 - 1;
		}

		if ( y0 == (double) (m_height - 1) )
		{
			y1 = (int) y0;
			y0 = y1 - 1;
		}

		if ( z0 == (double) (m_depth - 1) )
		{
			z1 = (int) z0;
			z0 = z1 - 1;
		}

		double xd = (x - (double) x0) / (double) (x1 - x0);
		double yd = (y - (double) y0) / (double) (y1 - y0);
		double zd = (z - (double) z0) / (double) (z1 - z0);

		// X interpolation
		double c00 = GetValue(x0, y0, z0)*(1.0f - xd)
			+ GetValue(x1, y0, z0)*xd;

		double c10 = GetValue(x0, y1, z0)*(1.0f - xd)
			+ GetValue(x1, y1, z0)*xd;

		double c01 = GetValue(x0, y0, z1)*(1.0f - xd)
			+ GetValue(x1, y0, z1)*xd;

		double c11 = GetValue(x0, y1, z1)*(1.0f - xd)
			+ GetValue(x1, y1, z1)*xd;

		// Y interpolation
		double c0 = c00*(1.0f - yd) + c10*yd;
		double c1 = c01*(1.0f - yd) + c11*yd;

		// Z interpolation
		double c = c0*(1.0f - zd) + c1*zd;

		return c;
	}

	double Volume::InterpolatedValue(lqc::Vector3f pos)
	{
		return InterpolatedValue(pos.x, pos.y, pos.z);
	}

	double Volume::TrilinearScalarFunction(lqc::Vector3f pos, lqc::Vector3f rayeye, lqc::Vector3f raydirection)
	{
		float fx = pos.x, fy = pos.y, fz = pos.z;

		fx = (fx - m_pmin.x) / abs(m_pmax.x - m_pmin.x) * m_width;
		fy = (fy - m_pmin.y) / abs(m_pmax.y - m_pmin.y) * m_height;
		fz = (fz - m_pmin.z) / abs(m_pmax.z - m_pmin.z) * m_depth;

		int x = (int) fx;
		int y = (int) fy;
		int z = (int) fz;

		if ( x == m_width ) --x;
		if ( y == m_height ) --y;
		if ( z == m_depth ) --z;

		//////////////////////////////////////////////////////////////////////////////
		//f(x, y, z) = c0 + c1*x + c2*y + c3*z + c4*x*y + c5*y*z + c6*x*z + c7*x*y*z//
		//////////////////////////////////////////////////////////////////////////////

		//cell coeficients
		float c[8];
		//scalar values of the cells
		float s[8];
		int m = 8, n = 8;
		unsigned int px[8] = {x, x + 1, x, x + 1, x, x + 1, x, x + 1};
		unsigned int py[8] = {y, y, y, y, y + 1, y + 1, y + 1, y + 1};
		unsigned int pz[8] = {z, z, z + 1, z + 1, z, z, z + 1, z + 1};
		for ( int i = 0; i < n; i++ )
			s[i] = (float) GetValue(px[i], py[i], pz[i]);

		//mallocs
		float **a = (float**) malloc(sizeof(float*)* m);
		for ( int i = 0; i < m; i++ )
		{
			a[i] = (float*) malloc(sizeof(float)* n);
			a[i][0] = 1;
			a[i][1] = (float) px[i];
			a[i][2] = (float) py[i];
			a[i][3] = (float) pz[i];
			a[i][4] = (float) px[i] * py[i];
			a[i][5] = (float) py[i] * pz[i];
			a[i][6] = (float) px[i] * pz[i];
			a[i][7] = (float) px[i] * py[i] * pz[i];
		}
		float *svdw = (float*) malloc(sizeof(float)* n);
		float **v = (float**) malloc(sizeof(float*)* n);
		for ( int i = 0; i < n; i++ )
			v[i] = (float*) malloc(sizeof(float)* n);

		//Make SVD on Matrix a
		lqc::dcksvd(a, m, n, svdw, v);

		//////////////////////////////////////////////////////////////
		//f(t) = w3*(t*t*t) + w2*(t*t) + w1*t + w0, t[tback, tfront]//
		//////////////////////////////////////////////////////////////

		//TODO
		float t = 0.0f;

		lqc::Vector3f e = rayeye;
		lqc::Vector3f d = raydirection;

		float w[4];
		w[0] = c[0] + c[1] * e.x + c[2] * e.y + c[4] * e.x*e.y + c[3] * e.z
			+ c[6] * e.x*e.z + c[5] * e.y*e.z + c[7] * e.x*e.y*e.z
			+ c[7] * d.x*d.y*e.z;
		w[1] = c[1] * d.x + c[2] * d.y + c[3] * d.z + c[4] * d.y*e.x + c[6] * d.z*e.x
			+ c[4] * d.x*e.y + c[5] * d.z*e.y + c[7] * d.z*e.x*e.y + c[6] * d.x*e.z + c[5] * d.y*e.z
			+ c[7] * d.y*e.x*e.z + c[7] * d.x*e.y*e.z;
		w[2] = c[4] * d.x*d.y + c[6] * d.x*d.z + c[5] * d.y*d.z + c[7] * d.y*d.z*e.x + c[7] * d.x*d.z*e.y;
		w[3] = c[7] * d.x*d.y*d.z;

		float f_t = w[3] * (t*t*t) + w[2] * (t*t) + w[1] * t + w[0];

		//frees
		for ( int i = 0; i < m; i++ )
			free(a[i]);
		free(a);

		for ( int i = 0; i < n; i++ )
			free(v[i]);
		free(v);

		free(svdw);

		// return f_t;
		return 0.0f;
	}

	bool Volume::IsOutOfBoundary(int x, int y, int z)
	{
		return !((x >= 0 && x < m_width)
						 && (y >= 0 && y < m_height)
						 && (z >= 0 && z < m_depth));
	}

	/////////////////////
	// Private Methods //
	/////////////////////

}