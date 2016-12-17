/// FastTFGenerator.h
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#ifndef FastTFGenerator_H
#define FastTFGenerator_H

#include "IATFGenerator.h"
#include "DerivativeMask.h"
#include <volrend\TransferFunction1D.h>
#include <iup.h>

/*************************/
/* Constants Definitions */
/*************************/

#define FAST_TFG_V_MAX 255
#define FAST_TFG_V_RANGE FAST_TFG_V_MAX + 1

class TransferFunction;

class FastTFGenerator : public IATFGenerator
{
public:
	FastTFGenerator (vr::ScalarField* scalarfield);

	~FastTFGenerator();

public:
	bool Init();

	bool ExtractTransferFunction();

	vr::TransferFunction* GetTransferFunction();

	void SetMainPlot(Ihandle * ih)
	{
		m_main_plot = ih;
	}

	void SetTransferFunctionPlot(Ihandle * ih)
	{
		m_tf_plot = ih;
	}

	void SetBoundaryFunctionPlot(Ihandle * ih)
	{
		m_bx_plot = ih;
	}

	float GetValue(UINT32 x, UINT32 y, UINT32 z);

private:
	float CalculateGradient(int x, int y, int z);

	float CalculateLaplacian(int x, int y, int z);

	bool CalculateVolumeDerivatives();

	bool GenerateHistogram();

	float GetBoundaryDistancies(float* x, unsigned char *v, int *n);

	unsigned int GetId(unsigned int x, unsigned int y, unsigned int z);

private:
	float* m_scalar_laplacian;

	float m_scalar_histogram[FAST_TFG_V_RANGE];
	float m_scalar_histogram_laplacian[FAST_TFG_V_RANGE];

	vr::TransferFunction1D* m_transfer_function;

	bool m_initialized;

	DerivativeMask m_derivativeMask;

	Ihandle* m_main_plot;
	Ihandle* m_tf_plot;
	Ihandle* m_bx_plot;
};

#endif