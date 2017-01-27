#ifndef VOLREND_TRANSFERFUNCTION1D_H
#define VOLREND_TRANSFERFUNCTION1D_H

#include <volrend/TransferFunction.h>
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
		virtual lqc::Vector4d Get (double value);

		virtual gl::GLTexture1D* GenerateTexture_RGBA ();

		void AddRGBControlPoint (TransferControlPoint rgb);
		void AddAlphaControlPoint (TransferControlPoint alpha);
		void ClearControlPoints();
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
		std::vector<TransferControlPoint> m_cpt_rgb;
		std::vector<TransferControlPoint> m_cpt_alpha;
		lqc::Vector4d* m_transferfunction;
		int m_tflenght;
		lqc::Vector3f* m_gradients;

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

		void SetBoundary(int boundary)
		{
			m_boundary = boundary;
		}

		/// <summary>
		/// Specifies the distance between a intensity value
		/// and its closest boundary. Thus, the input arrays'
		/// size must range from 2 to 256. Any array content
		/// whose index is greater than 255, it'll be ignored.
		/// </summary>
		/// <param name="values">The values array.</param>
		/// <param name="distances">The distances to the closest boundaries.</param>
		/// <param name="n">The input arrays' size.</param>
    void SetClosestBoundaryDistances(int* values, double* distances, double* h, const int& n);

		void SetAlphaValues(int* values, double* alphas, const int& n);
		
		void SetPeakPoints(int* peaks, double* values, const int& n);

    void SetGaussianFunction()
    {
      m_gaussian_bx = true;
    }

    void SetTriangularFunction()
    {
      m_gaussian_bx = false;
    }

	private:
    double CenteredTriangleFunction(double max, double base, double center, const int& v);
		double CenteredGaussianFunction(double max, double sigma, double u, const int& v);

	private:
		/// <summary>
		/// It storages the values setted by
		/// SetClosestBoundaryDistances.
		/// </summary>
		int* m_indexes;
		/// <summary>
		/// It storages the distances setted by 
		/// SetClosestBoundaryDistances.
		/// </summary>
		double* m_values;
    double* m_center;
		bool m_direct_tf;
		bool m_peakbased_tf;
		bool m_gordon_tf;
		int m_thickness;
		int m_boundary;
		int m_values_size;
    int m_gaussian_bx;
	};

}

#endif