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
		virtual lqc::Vector4d Get (double value);

    virtual gl::GLTexture2D* GenerateTexture_RGBA();

		void AddRGBControlPoint (lqc::Vector3f rgb, int v, int g);
    void AddAlphaControlPoint(double alpha, int v, int g);
		void ClearRGBControlPoints();
    void ClearAlphaControlPoints();

		lqc::Vector3f GetRGBPointOnSpline (float s);
		lqc::Vector3f GetAlphaPointOnSpline (float s);

		//If we don't have a file with the values of the TF, we need to compute the TF
		void Build (TFInterpolationType type);

		void PrintControlPoints ();
		void PrintTransferFunction ();

		TFInterpolationType m_interpolation_type;
		bool m_built;
	private:
    
    lqc::Vector4d m_transferfunction[MAX_V][MAX_V];
    double m_v0, m_v1;

	public:
		/// <summary>
		/// Generates a transfer function file at a given path.
		/// If a file with the same path already exists, it'll
		/// be replaced.
		/// </summary>
		/// <returns>Returns true if the transfer function can
		/// be generated, false otherwise.</returns>
		bool GenerateGordonBased();

		bool Generate();

		void SetBoundaryThickness(int thickness)
		{
			m_thickness = thickness;
		}

    void SetClosestBoundaryDistances(double** distances);

    void SetGaussianFunction()
    {
      m_gaussian_bx = true;
    }

    void SetTriangularFunction()
    {
      m_gaussian_bx = false;
    }

	private:
    double CenteredTriangleFunction(double max, double base, double center, const int& v, const int& g);
    double CenteredGaussianFunction(double max, double sigma, double u, const int& v, const int& g);

	private:
    double** m_distances;
		int m_thickness;
    int m_gaussian_bx;
    int m_width, m_height;
	};

}

#endif