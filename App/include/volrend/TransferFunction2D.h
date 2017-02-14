#ifndef VOLREND_TRANSFERFUNCTION2D_H
#define VOLREND_TRANSFERFUNCTION2D_H

#include <volrend/TransferFunction.h>
#include <glutils/GLTexture2d.h>
#include <vector>
#include <iostream>
#include <iup.h>
#include "../../ATFGenerator/src/AutomaticTransferFunction/PredictionMap.h"

namespace vr
{
	class TransferFunction2D : public TransferFunction
	{
	public:
		TransferFunction2D (double v0 = 0.0, double v1 = 256.0);
		~TransferFunction2D ();

		virtual const char* GetNameClass ();
		virtual lqc::Vector4d Get (double value);

		virtual gl::GLTexture2D* GenerateTexture_RGBA();

		void AddRGBControlPoint (lqc::Vector3f rgb, int v, int g);
		void AddAlphaControlPoint(double alpha, int v, int g);
		void ClearRGBControlPoints();
		void ClearAlphaControlPoints();

		lqc::Vector3f GetRGBPointOnSpline (float s);
		lqc::Vector3f GetAlphaPointOnSpline (float s);

		//If we don't have a file with the values of the TF, we need to compute the TF
		void Interpolate ();

		void PrintControlPoints ();
		void PrintTransferFunction ();
		
		virtual bool GenerateGordonBased();

		virtual bool Generate();

    void SetClosestBoundaryDistances(PredictionMap<double, DoubleCell>* distances)
    {
      m_distances = distances;
    }

    void SetSigma(double sigma)
    {
      m_sigma = sigma;
    }

	private:
		double CenteredTriangleFunction(double max, double base, double center, const int& v, const int& g);
		double CenteredGaussianFunction(double max, double sigma, double u, const int& v, const int& g);
    
    struct TFInfo
    {
      lqc::Vector3d rgb;
      double alpha;
      TFInfo() : rgb(), alpha(0.0f) {}
    };

	private:
    TFInterpolationType m_interpolation_type;
    bool m_has_rgb;
    bool m_has_alpha;
    TFInfo m_transferfunction[MAX_V][MAX_V];
    PredictionMap<double, DoubleCell>* m_distances;
    double m_sigma;
	};

}

#endif