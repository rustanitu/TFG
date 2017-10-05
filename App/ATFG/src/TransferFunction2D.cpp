#include "TransferFunction2D.h"

#include <fstream>
#include <cstdlib>
#include <iup_mglplot.h>

namespace vr
{
  TransferFunction2D::TransferFunction2D(double v0, double v1)
    : m_has_rgb(false), m_has_alpha(false)
	{
		printf("TransferFunction2D criado.\n");
		ClearRGBControlPoints();
	}

	TransferFunction2D::~TransferFunction2D ()
	{
		printf("TransferFunction2D destruido.\n");
    delete m_distmap;
	}

	const char* TransferFunction2D::GetNameClass ()
	{
		return "TrasnferFunction2D";
	}

  void TransferFunction2D::AddRGBControlPoint(const glm::vec3& rgb, int v, int g)
	{ 
		m_transferfunction[v][g].rgb.x = rgb.x;
		m_transferfunction[v][g].rgb.y = rgb.y;
		m_transferfunction[v][g].rgb.z = rgb.z;
    m_has_rgb = true;
	}

	void TransferFunction2D::AddAlphaControlPoint(double alpha, int v, int g)
	{
		m_transferfunction[v][g].alpha = alpha;
    m_has_alpha = true;
	}

	void TransferFunction2D::ClearRGBControlPoints ()
	{
    if (!m_has_rgb)
      return;

    for (int v = 0; v < MAX_V; ++v)
		{
      for (int g = 0; g < MAX_V; ++g)
			{
				m_transferfunction[v][g].rgb.x = 0.0f;
				m_transferfunction[v][g].rgb.y = 0.0f;
				m_transferfunction[v][g].rgb.z = 0.0f;
			}
		}

    m_has_rgb = false;
	}

	void TransferFunction2D::ClearAlphaControlPoints()
	{
    if (!m_has_alpha)
      return;

		for (int v = 0; v < MAX_V; ++v) {
			for (int g = 0; g < MAX_V; ++g) {
        m_transferfunction[v][g].alpha = 0.0f;
			}
		}

    m_has_alpha = false;
	}

	gl::GLTexture2D* TransferFunction2D::GenerateTexture_RGBA ()
	{
		printf("TransferFunction2D: GenerateTexture_RGBA.\n");

		if (m_transferfunction)
		{
			gl::GLTexture2D* ret = new gl::GLTexture2D (MAX_V, MAX_V);
			ret->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
			float* data = new float[MAX_V * MAX_V * 4];
			int c = 0;
			for (int v = 0; v < MAX_V; v++)
			{
				for (int g = 0; g < MAX_V; g++)
				{
					data[c++] = (float)(m_transferfunction[v][g].rgb.r);
					data[c++] = (float)(m_transferfunction[v][g].rgb.g);
					data[c++] = (float)(m_transferfunction[v][g].rgb.b);
          data[c++] = (float)(m_transferfunction[v][g].alpha);
				}
			}
			ret->SetData ((void*)data, GL_RGBA32F, GL_RGBA, GL_FLOAT);
			delete[] data;
			return ret;
		}
		return NULL;
	}

  void TransferFunction2D::Interpolate()
	{
	}

	void TransferFunction2D::PrintControlPoints ()
	{
		/*
		printf ("Print Transfer Function: Control Points\n");
		int rgb_pts = (int)m_rgb.size ();
		printf ("- Printing the RGB Control Points\n");
		printf ("  Format: \"Number: Red Green Blue, Isovalue\"\n");
		for (int i = 0; i < rgb_pts; i++)
		{
			printf ("  %d: %.2f %.2f %.2f, %d\n", i + 1, m_rgb[i].m_color.x, m_rgb[i].m_color.y, m_rgb[i].m_color.z, m_rgb[i].m_isoValue);
		}
		printf ("\n");

		int alpha_pts = (int)m_alpha.size ();
		printf ("- Printing the Alpha Control Points\n");
		printf ("  Format: \"Number: Alpha, Isovalue\"\n");
		for (int i = 0; i < alpha_pts; i++)
		{
			printf ("  %d: %.2f, %d\n", i + 1, m_alpha[i].m_color.w, m_alpha[i].m_isoValue);
		}
		printf ("\n");
		*/
	}

	void TransferFunction2D::PrintTransferFunction ()
	{
		printf ("Print Transfer Function: Control Points\n");
		printf ("  Format: \"IsoValue: Red Green Blue, Alpha\"\n");
		for (int i = 0; i < MAX_V; i++)
		{
			for (int j = 0; j < MAX_V; j++) {
        printf("[%d][%d]: %.2f %.2f %.2f, %.2f\n", i, j, m_transferfunction[i][j].rgb.x
          , m_transferfunction[i][j].rgb.y, m_transferfunction[i][j].rgb.z, m_transferfunction[i][j].alpha);
			}
		}
	}

	bool TransferFunction2D::Generate()
	{
    assert(m_distmap);

		printf("TransferFunction2D: GenerateGordonBased (from ATFG).\n");

		ClearAlphaControlPoints();
		double* data = new double[MAX_V*MAX_V];

    for (int i = 0; i < MAX_V; ++i)
    {
      for (int j = 0; j < MAX_V; ++j)
      {
        double x = m_distmap->GetValue(i, j);
        
        double a = 0.0f;
        if (m_gaussian_bx)
          a = CenteredGaussianFunction(x, 1.0f, 1.0f / (0.5f * m_thickness), m_sigma);
        else
          a = CenteredTriangleFunction(x, 1.0f, 1.0f / m_thickness, m_sigma);
        
        if (m_distmap->GetValue(i, j) == -DBL_MAX)
          a = 1;

        AddAlphaControlPoint(a, i, j);
        data[i + MAX_V*j] = a;
      }
    }

		IupSetAttribute(m_tf_plot, "CLEAR", "YES");
		int index = IupMglPlotNewDataSet(m_tf_plot, 1);
		IupMglPlotSetData(m_tf_plot, index, data, MAX_V, MAX_V, 1);
		IupSetAttribute(m_tf_plot, "DS_MODE", "PLANAR_SURFACE");
		IupSetAttribute(m_tf_plot, "LEGEND", "NO");
		//IupSetAttribute(m_tf_plot, "OPENGL", "YES");
		IupSetAttribute(m_tf_plot, "LIGHT", "NO");
		IupSetAttribute(m_tf_plot, "AXS_ZAUTOMIN", "NO");
		IupSetAttribute(m_tf_plot, "AXS_ZMIN", "-0.001");
		IupSetAttribute(m_tf_plot, "AXS_ZAUTOMAX", "NO");
		IupSetAttribute(m_tf_plot, "AXS_ZMAX", "1.001");
		IupSetAttribute(m_tf_plot, "AXS_XLABEL", "Scalar Value");
		IupSetAttribute(m_tf_plot, "AXS_YLABEL", "Gradient");
		IupSetAttribute(m_tf_plot, "AXS_ZLABEL", "Alpha");
    //IupSetAttribute(m_tf_plot, "COLORSCHEME", "kw");
    //IupSetAttribute(m_tf_plot, "COLORSCHEME", "kkry");
		//IupSetAttribute(m_tf_plot, "ROTATE", "0:0:-90");
		IupSetAttribute(m_tf_plot, "REDRAW", NULL);

		delete[] data;

		return true;
	}
}