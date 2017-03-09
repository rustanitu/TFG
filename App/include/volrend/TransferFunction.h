#ifndef VOLREND_TRANSFERFUNCTION_H
#define VOLREND_TRANSFERFUNCTION_H

#include <iostream>
#include <math/MUtils.h>
#include <vector>
#include <iup.h>
#include <glutils/GLTexture1D.h>
#include <volrend/PredictionMap.h>

#define MAX_V 256
#define PI 3.1415926535897932384626433832795
#define GAUSS_PERC_AT_SIGMA 0.60653065971

namespace vr
{
	class TransferFunction
	{
	public:
    TransferFunction();
    ~TransferFunction();

		virtual const char* GetNameClass () = 0;

    virtual bool Generate() { return false; }

    virtual glm::dvec4 Get(double p) { return glm::dvec4(0.0f); }
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

    void SetClosestBoundaryDistances(PredictionMap<double, DoubleCell>* distmap)
    {
      if (m_distmap)
        m_distmap->CleanUp();
      delete m_distmap;

      m_distmap = distmap;
    }

    static double CenteredTriangleFunction(double x, double max, double base, double center);

    static double CenteredGaussianFunction(double x, double max, double base, double u);

    void SetSigma(double sigma)
    {
      m_sigma = sigma;
    }

    struct TFInfo
    {
      glm::vec3 rgb;
      double alpha;
      TFInfo() : rgb(), alpha(0.0f) {}
    };
		
	protected:
		std::string m_name;
    Ihandle * m_tf_plot;
    int m_thickness;
    int m_gaussian_bx;
    PredictionMap<double, DoubleCell>* m_distmap;
    double m_sigma;
	};
}

#endif