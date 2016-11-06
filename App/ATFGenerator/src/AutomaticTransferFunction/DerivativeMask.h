#ifndef ATFG_DERIVATIVE_MASK
#define ATFG_DERIVATIVE_MASK

static class DerivativeMask
{
public:
	DerivativeMask(const int& n);

	void GetGradient(const int& x, const int& y, const int& z, float* gx, float* gy, float* gz);
	void GetLaplacian(const int& x, const int& y, const int& z, float* lx, float* ly, float* lz);

private:
	unsigned int GetId(const int& x, const int& y, const int& z);
	void GenerateGradientMask();
	void GenerateLaplacianMask();
private:
	int m_n;
	float *m_gaussian_kernel;
	float *m_gradient_kernel;
	float *m_laplacian_kernel;
	float *m_gradient_mask;
	float *m_laplacian_mask;
};

#endif