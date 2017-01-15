#ifndef ATFG_DERIVATIVE_MASK
#define ATFG_DERIVATIVE_MASK

static class DerivativeMask
{
public:
	DerivativeMask(const int& n);

	void GetGradient(const int& x, const int& y, const int& z, float* gx, float* gy, float* gz);
	void GetLaplacian(const int& x, const int& y, const int& z, float* lx, float* ly, float* lz);

  float GetDxAt(const int& x, const int& y, const int& z) const;
  float GetDyAt(const int& x, const int& y, const int& z) const;
  float GetDzAt(const int& x, const int& y, const int& z) const;
  float GetDxdxAt(const int& x, const int& y, const int& z) const;
  float GetDxdyAt(const int& x, const int& y, const int& z) const;
  float GetDxdzAt(const int& x, const int& y, const int& z) const;
  float GetDydxAt(const int& x, const int& y, const int& z) const;
  float GetDydyAt(const int& x, const int& y, const int& z) const;
  float GetDydzAt(const int& x, const int& y, const int& z) const;
  float GetDzdxAt(const int& x, const int& y, const int& z) const;
  float GetDzdyAt(const int& x, const int& y, const int& z) const;
  float GetDzdzAt(const int& x, const int& y, const int& z) const;

private:
	unsigned int GetId(const int& x, const int& y, const int& z) const;
	void GenerateMasks();
	void GenerateGradientMask();
	void GenerateLaplacianMask();
private:
	int m_n;
	float* m_h;   // Normal or gaussian kernel
	float* m_hl;  // First derivative kernel
  float* m_hll; // Second derivative kernel
	float* m_gradient_mask;
	float* m_laplacian_mask;
  
  //
  float* m_dx_mask;
  float* m_dy_mask;
  float* m_dz_mask;
  
  float* m_dxdx_mask;
  float* m_dxdy_mask;
  float* m_dxdz_mask;
  
  float* m_dydy_mask;
  float* m_dydz_mask;

  float* m_dzdz_mask;
};

#endif