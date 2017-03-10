#include "TransferFunction1D.h"

#include <fstream>
#include <cstdlib>
#include <iup_plot.h>

namespace vr
{
	TransferFunction1D::TransferFunction1D (double v0, double v1)
		: m_boundary(0)
	{
		printf("TransferFunction1D criado.\n");
	}

	TransferFunction1D::~TransferFunction1D ()
	{
		printf("TransferFunction1D destruido.\n");
	}

	const char* TransferFunction1D::GetNameClass ()
	{
		return "TrasnferFunction1D";
	}

  void TransferFunction1D::AddRGBControlPoint(const glm::vec3& rgb, int v)
	{
    m_transferfunction[v].rgb.x = rgb.x;
    m_transferfunction[v].rgb.y = rgb.y;
    m_transferfunction[v].rgb.z = rgb.z;
    m_has_rgb = true;
	}

  void TransferFunction1D::AddAlphaControlPoint(double alpha, int v)
	{
    m_transferfunction[v].alpha = alpha;
    m_has_alpha = true;
	}

  void TransferFunction1D::ClearRGBControlPoints()
  {
    if (!m_has_rgb)
      return;

    for (int v = 0; v < MAX_V; ++v)
    {
      m_transferfunction[v].rgb.x = 0.0f;
      m_transferfunction[v].rgb.y = 0.0f;
      m_transferfunction[v].rgb.z = 0.0f;
    }

    m_has_rgb = false;
  }

  void TransferFunction1D::ClearAlphaControlPoints()
  {
    if (!m_has_alpha)
      return;

    for (int v = 0; v < MAX_V; ++v)
    {
      m_transferfunction[v].alpha = 0.0f;
    }

    m_has_alpha = false;
  }

	gl::GLTexture1D* TransferFunction1D::GenerateTexture_RGBA ()
	{
		printf("TransferFunction1D: GenerateTexture_RGBA.\n");

		if (m_transferfunction)
		{
			gl::GLTexture1D* ret = new gl::GLTexture1D (MAX_V);
			ret->GenerateTexture (GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_BORDER);
      float* data = new float[MAX_V * 4];
      for (int i = 0; i < MAX_V; i++)
			{
				data[(i * 4)] = (float)(m_transferfunction[i].rgb.x);
				data[(i * 4) + 1] = (float)(m_transferfunction[i].rgb.y);
				data[(i * 4) + 2] = (float)(m_transferfunction[i].rgb.z);
				data[(i * 4) + 3] = (float)(m_transferfunction[i].alpha);
			}
			ret->SetData ((void*)data, GL_RGBA32F, GL_RGBA, GL_FLOAT);
			delete[] data;
			return ret;
		}
		return NULL;
	}

	void TransferFunction1D::PrintControlPoints ()
	{
		/*printf ("Print Transfer Function: Control Points\n");
		int rgb_pts = (int)m_cpt_rgb.size ();
		printf ("- Printing the RGB Control Points\n");
		printf ("  Format: \"Number: Red Green Blue, Isovalue\"\n");
		for (int i = 0; i < rgb_pts; i++)
		{
			printf ("  %d: %.2f %.2f %.2f, %d\n", i + 1, m_cpt_rgb[i].m_color.x, m_cpt_rgb[i].m_color.y, m_cpt_rgb[i].m_color.z, m_cpt_rgb[i].m_isoValue);
		}
		printf ("\n");

		int alpha_pts = (int)m_cpt_alpha.size ();
		printf ("- Printing the Alpha Control Points\n");
		printf ("  Format: \"Number: Alpha, Isovalue\"\n");
		for (int i = 0; i < alpha_pts; i++)
		{
			printf ("  %d: %.2f, %d\n", i + 1, m_cpt_alpha[i].m_color.w, m_cpt_alpha[i].m_isoValue);
		}
		printf ("\n");*/
	}

	void TransferFunction1D::PrintTransferFunction ()
	{
		//printf ("Print Transfer Function: Control Points\n");
		//printf ("  Format: \"IsoValue: Red Green Blue, Alpha\"\n");
		//for (int i = 0; i < m_tflenght; i++)
		//{
		//	printf ("%d: %.2f %.2f %.2f, %.2f\n", i, m_transferfunction[i].x
		//		, m_transferfunction[i].y, m_transferfunction[i].z, m_transferfunction[i].w);
		//}
	}

  bool TransferFunction1D::Generate()
	{
    assert(m_distmap);

		printf("TransferFunction1D: Generate (from ATFG).\n");

		ClearAlphaControlPoints();
		IupSetAttribute(m_tf_plot, "CLEAR", "YES");
		IupPlotBegin(m_tf_plot, 0);

		double amax = 1.0f;

		double last_a = 0.0f;
		int b = 0;
		bool finished = true;

		// Assign opacity to transfer function
    for (int i = 0; i < MAX_V; ++i)
		{
			int value = i;
			double x = m_distmap->GetValue(0,i);
			double a = 0.0f;
			if (m_gaussian_bx)
				a = CenteredGaussianFunction(x, amax, 1.0f / m_thickness, 0.0f);
			else
				a = CenteredTriangleFunction(x, amax, 1.0f / m_thickness, 0.0f);

			if ( m_boundary != 0 )
			{
				if ( last_a <= 0.1f && a > 0.1f )
				{
					++b;
					finished = false;
					if ( b == m_boundary && i - 1 >= 0 )
					{
						AddAlphaControlPoint(0.0f, i - 1);
						IupPlotAdd(m_tf_plot, value - 1, 0.0f);
					}
				}
				if ( a <= 0.1f )
				{
					finished = true;
					AddAlphaControlPoint(0.0f, value);
					IupPlotAdd(m_tf_plot, value, 0.0f);
				}
				last_a = a;
				if ( b != m_boundary || finished )
					continue;
			}

			AddAlphaControlPoint(a, value);
			IupPlotAdd(m_tf_plot, value, a);
		}

		IupPlotEnd(m_tf_plot);
		IupSetAttribute(m_tf_plot, "DS_NAME", "Transfer Function");
		IupSetAttribute(m_tf_plot, "DS_COLOR", "0 0 0");
		IupSetAttribute(m_tf_plot, "REDRAW", "YES");

		return true;
	}
}