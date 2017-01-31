#ifndef VOLREND_TRANSFERFUNCTION_H
#define VOLREND_TRANSFERFUNCTION_H

#include <iostream>
#include <math/Vector3.h>
#include <math/Vector4.h>
#include <vector>
#include <iup.h>
#include <glutils/GLTexture1D.h>

#define MAX_V 256
#define PI 3.1415926535897932384626433832795
#define GAUSS_PERC_AT_SIGMA 0.60653065971

namespace vr
{
	class ScalarField;

	enum TFInterpolationType
	{
		LINEAR,
		CUBIC
	};

	enum TFFormatType
	{
		LQC
	};
	
	class TransferControlPoint
	{
	public:
		TransferControlPoint (unsigned int r, unsigned int g, unsigned int b, int isovalue);
		TransferControlPoint (double r, double g, double b, int isovalue);
		TransferControlPoint (double alpha, int isovalue);
		TransferControlPoint()
		{
			m_color = lqc::Vector4d::Zero();
			m_isoValue = 0;
		}

		lqc::Vector3d operator -(const TransferControlPoint& v)  
		{  
			lqc::Vector4d ret;
			ret = this->m_color - v.m_color;

			return lqc::Vector3d (ret.x, ret.y, ret.z);
		}

		lqc::Vector3d operator +(const TransferControlPoint& v)
		{  
			lqc::Vector4d ret;
			ret = this->m_color + v.m_color;

			return lqc::Vector3d (ret.x, ret.y, ret.z);
		}

		lqc::Vector3d operator +(const lqc::Vector3d& v)
		{  
			return lqc::Vector3d (this->m_color.x + v.x, this->m_color.y + v.y, this->m_color.z + v.z);
		}

		lqc::Vector4d m_color;
		int m_isoValue;
	};
		
	class Cubic
	{
	public:
		Cubic ();
		Cubic (lqc::Vector4d in1, lqc::Vector4d in2, lqc::Vector4d in3, lqc::Vector4d in4);
		~Cubic ();

		lqc::Vector4d GetPointOnSpline (double s);
		static Cubic* CalculateCubicSpline (int n, std::vector<TransferControlPoint> v);

		lqc::Vector4d a, b, c, d;
	};
	Cubic* CalculateCubicSpline (int n, std::vector<TransferControlPoint> v);

	class TransferFunction
	{
	public:
    TransferFunction () : m_gaussian_bx(true), m_thickness(1) {}
		~TransferFunction () {}

		virtual const char* GetNameClass () = 0;
		virtual lqc::Vector4d Get (double value) = 0;

    virtual bool GenerateGordonBased() { return false; }

    virtual bool Generate() { return false; }

		virtual gl::GLTexture* GenerateTexture_RGBA () { return NULL; }
		
		std::string GetName () { return m_name; }
		void SetName (std::string name) { m_name = name; }

		void SetTransferFunctionPlot(Ihandle * ih)
		{
			m_tf_plot = ih;
    }

    void SetBoundaryThickness(int thickness)
    {
      m_thickness = thickness;
    }

    void SetGaussianFunction()
    {
      m_gaussian_bx = true;
    }

    void SetTriangularFunction()
    {
      m_gaussian_bx = false;
    }
		
	protected:
		std::string m_name;
    Ihandle * m_tf_plot;
    int m_thickness;
    int m_gaussian_bx;
	};

	/*class TransferFunction1D : public TransferFunction
	{
	public:
		TransferFunction1D (double v0 = 0.0, double v1 = 256.0);
		~TransferFunction1D ();

		virtual const char* GetNameClass ();
		virtual lqc::Vector4d Get (double value);

		virtual GLTexture1D* GenerateTexture_RGBA ();

		void AddRGBControlPoint (TransferControlPoint rgb);
		void AddAlphaControlPoint (TransferControlPoint alpha);
		void ClearControlPoints ();

		Vector3f GetRGBPointOnSpline (float s);
		Vector3f GetAlphaPointOnSpline (float s);

		//If we don't have a file with the values of the TF, we need to compute the TF
		void Build (TFInterpolationType type);

		void PrintControlPoints ();
		void PrintTransferFunction ();

		void Save (char* filename, TFFormatType format = TFFormatType::LQC);
		bool Load (std::string filename, TFFormatType format = TFFormatType::LQC);

		bool m_built;
	private:
		std::vector<TransferControlPoint> m_cpt_rgb;
		std::vector<TransferControlPoint> m_cpt_alpha;
		Vector4f* m_transferfunction;
		int m_tflenght;
		Vector3f* m_gradients;

		double m_v0, m_v1;
	};*/
}

#endif