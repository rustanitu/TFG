#ifndef ATFG_DERIVATIVE_MASK
#define ATFG_DERIVATIVE_MASK

class DerivativeMask
{
public:
	DerivativeMask(const int& n);

	void GetGradient(const int& x, const int& y, const int& z, double* gx, double* gy, double* gz);
	void GetLaplacian(const int& x, const int& y, const int& z, double* lx, double* ly, double* lz);

	double GetDxAt(const int& x, const int& y, const int& z) const;
	double GetDyAt(const int& x, const int& y, const int& z) const;
	double GetDzAt(const int& x, const int& y, const int& z) const;
	double GetDxdxAt(const int& x, const int& y, const int& z) const;
	double GetDxdyAt(const int& x, const int& y, const int& z) const;
	double GetDxdzAt(const int& x, const int& y, const int& z) const;
	double GetDydxAt(const int& x, const int& y, const int& z) const;
	double GetDydyAt(const int& x, const int& y, const int& z) const;
	double GetDydzAt(const int& x, const int& y, const int& z) const;
	double GetDzdxAt(const int& x, const int& y, const int& z) const;
	double GetDzdyAt(const int& x, const int& y, const int& z) const;
	double GetDzdzAt(const int& x, const int& y, const int& z) const;

private:
	unsigned int GetId(const int& x, const int& y, const int& z) const;
	void GenerateMasks();
	void GenerateGradientMask();
	void GenerateLaplacianMask();
private:
	int m_n;
	double* m_h;   // Normal or gaussian kernel
	double* m_hl;  // First derivative kernel
	double* m_hll; // Second derivative kernel
	double* m_gradient_mask;
	double* m_laplacian_mask;
	
	//
	double* m_dx_mask;
	double* m_dy_mask;
	double* m_dz_mask;
	
	double* m_dxdx_mask;
	double* m_dxdy_mask;
	double* m_dxdz_mask;
	
	double* m_dydy_mask;
	double* m_dydz_mask;

	double* m_dzdz_mask;
};

#endif