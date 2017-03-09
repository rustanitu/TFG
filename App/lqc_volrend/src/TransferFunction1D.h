#ifndef VOLREND_TRANSFERFUNCTION1D_H
#define VOLREND_TRANSFERFUNCTION1D_H

#include "TransferFunction.h"
#include <glutils/GLTexture1D.h>
#include <vector>
#include <iostream>
#include <iup.h>

namespace vr
{
	class TransferFunction1D : public TransferFunction
	{
	public:
		TransferFunction1D (double v0 = 0.0, double v1 = 256.0);
		~TransferFunction1D ();

		virtual const char* GetNameClass ();
		virtual gl::GLTexture1D* GenerateTexture_RGBA ();

    void AddRGBControlPoint(const glm::vec3& rgb, int v);
    void AddAlphaControlPoint(double alpha, int v);
    void ClearRGBControlPoints();
		void ClearAlphaControlPoints();

		void PrintControlPoints ();
		void PrintTransferFunction ();

		virtual bool Generate();

		void SetBoundary(int boundary)
		{
			m_boundary = boundary;
		}

	private:
		int m_boundary;
    bool m_has_rgb;
    bool m_has_alpha;
    TFInfo m_transferfunction[MAX_V];
	};

}

#endif