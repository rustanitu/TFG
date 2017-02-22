#ifndef VOLREND_TRANSFERFUNCTION2D_H
#define VOLREND_TRANSFERFUNCTION2D_H

#include <volrend/TransferFunction.h>
#include <glutils/GLTexture2d.h>
#include <vector>
#include <iostream>
#include <iup.h>

namespace vr
{
	class TransferFunction2D : public TransferFunction
	{
	public:
		TransferFunction2D (double v0 = 0.0, double v1 = 256.0);
		~TransferFunction2D ();

		virtual const char* GetNameClass ();

		virtual gl::GLTexture2D* GenerateTexture_RGBA();

    void AddRGBControlPoint(const glm::vec3& rgb, int v, int g);
		void AddAlphaControlPoint(double alpha, int v, int g);
		void ClearRGBControlPoints();
		void ClearAlphaControlPoints();

		void Interpolate ();

		void PrintControlPoints ();
		void PrintTransferFunction ();
		
		virtual bool Generate();

	private:
    bool m_has_rgb;
    bool m_has_alpha;
    TFInfo m_transferfunction[MAX_V][MAX_V];
	};

}

#endif