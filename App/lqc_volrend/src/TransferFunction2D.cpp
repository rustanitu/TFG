#include <volrend/TransferFunction2D.h>

#include <fstream>
#include <cstdlib>
#include <iup_mglplot.h>

namespace vr
{
  TransferFunction2D::TransferFunction2D(double v0, double v1)
    : m_interpolation_type(TFInterpolationType::LINEAR)
    , m_sigma(0.0f), m_has_rgb(false), m_has_alpha(false)
    , m_distances(NULL), m_indexes(NULL)
	{
		printf("TransferFunction2D criado.\n");
		ClearRGBControlPoints();
	}

	TransferFunction2D::~TransferFunction2D ()
	{
		printf("TransferFunction2D destruido.\n");
    delete m_distances;
	}

	const char* TransferFunction2D::GetNameClass ()
	{
		return "TrasnferFunction2D";
	}

	void TransferFunction2D::AddRGBControlPoint(lqc::Vector3f rgb, int v, int g)
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
					data[c++] = (float)(m_transferfunction[v][g].rgb.x);
					data[c++] = (float)(m_transferfunction[v][g].rgb.y);
					data[c++] = (float)(m_transferfunction[v][g].rgb.z);
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

	lqc::Vector4d TransferFunction2D::Get (double value)
	{
		return lqc::Vector4d(0);
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

	/*
	void TransferFunction2D::Save (char* filename, TFFormatType format)
	{
	std::string filesaved;
	filesaved.append (RESOURCE_LIBLQC_PATH);
	filesaved.append ("TransferFunctions/");
	filesaved.append (filename);
	if (format == TFFormatType::LQC)
	{
	filesaved.append (".tf2D");
	std::ofstream myfile (filesaved.c_str ());
	if (myfile.is_open ())
	{
	myfile << 0 << "\n";
	myfile << (int)m_rgb.size () << "\n";
	for (int i = 0; i < (int)m_rgb.size (); i++)
	{
	myfile << m_rgb[i].m_color.x << " " <<
	m_rgb[i].m_color.y << " " <<
	m_rgb[i].m_color.z << " " <<
	m_rgb[i].m_isoValue << " " << "\n";
	}
	myfile << (int)m_alpha.size () << "\n";
	for (int i = 0; i < (int)m_alpha.size (); i++)
	{
	myfile << m_alpha[i].m_color.w << " " <<
	m_alpha[i].m_isoValue << " " << "\n";
	}
	myfile.close ();
	printf ("lqc: Transfer Function 2D Control Points Saved!\n");
	}
	else
	{
	printf ("lqc: Error on opening file at VRTransferFunction::Save().\n");
	}
	}
	}

	bool TransferFunction2D::Load (std::string filename, TFFormatType format)
	{
	std::string filesaved;
	filesaved.append (RESOURCE_LIBLQC_PATH);
	filesaved.append ("TransferFunctions/");
	filesaved.append (filename);
	if (format == TFFormatType::LQC)
	{
	filesaved.append (".tf2D");
	std::ifstream myfile (filesaved.c_str ());
	if (myfile.is_open ())
	{
	int init;
	myfile >> init;

	int cpt_rgb_size;
	myfile >> cpt_rgb_size;
	float r, g, b, a;
	int isovalue;
	for (int i = 0; i < cpt_rgb_size; i++)
	{
	myfile >> r >> g >> b >> isovalue;
	m_rgb.push_back (TransferControlPoint (r, g, b, isovalue));
	}

	int cpt_alpha_size;
	myfile >> cpt_alpha_size;
	for (int i = 0; i < cpt_alpha_size; i++)
	{
	myfile >> a >> isovalue;
	m_alpha.push_back (TransferControlPoint (a, isovalue));
	}
	myfile.close ();
	printf ("lqc: Transfer Function 2D Control Points Loaded!\n");
	return true;
	}
	else
	printf ("lqc: Error on opening file at VRTransferFunction::AddControlPointsReadFile().\n");
	}
	return false;
	}*/

  double TransferFunction2D::CenteredTriangleFunction(double x, double max, double base, double center)
	{
		//  boundary center
		//         .
		//        / \       |
		//       / | \      |
		//      /  |  \     |
		//     /   |   \    | max
		//    /    |    \   |
		//   /     |     \  |
		//  /      |      \ |
		//  ---- center ++++ 
		// |-------|-------|
		//       base

		double a = 0.0f;
    if (x >= -base && x <= base)
    {
      if (x >= center && center < base)
      {
        a = -(max * x) / (base - center);
        a += (max * base) / (base - center);
      }
      else
      {
        a = (max * x) / (base + center);
        a += (max * base) / (base + center);
      }
    }

    return fmin(a, 1.0f);
	}

	double TransferFunction2D::CenteredGaussianFunction(double x, double max, double base, double u)
	{
		double sigma = base / 3.0f;
    double gauss = max * exp(((-(x - u)*(x - u)) / (2 * sigma * sigma)));
    return fmin(gauss, 1.0f);
	}

	/// <summary>
	/// Generates a transfer function file at a given path.
	/// If a file with the same path already exists, it'll
	/// be replaced.
	/// </summary>
	/// <returns>Returns true if the transfer function can
	/// be generated, false otherwise.</returns>
	bool TransferFunction2D::GenerateGordonBased()
	{
		printf("TransferFunction2D: GenerateGordonBased (from ATFG).\n");

    PredictionMap<double, DoubleCell> map(MAX_V, MAX_V);
    if (!map.Init())
      return false;

    for (int k = 0; k < m_distances->size(); ++k)
    {
      double x = m_distances->at(k);
      std::pair<int, int> pair = m_indexes->at(k);
      int i = pair.first;
      int j = pair.second;

      double a = 0.0f;
      if (m_gaussian_bx)
        a = CenteredGaussianFunction(x, 1.0f, 1.0f / m_thickness, m_sigma);
      else
        a = CenteredTriangleFunction(x, 1.0f, 1.0f / m_thickness, m_sigma);
        
      map.SetValue(a, i, j);
    }

    map.PredictWithInverseDistanceWeighting(1.8f);
		ClearAlphaControlPoints();
		double* data = new double[MAX_V*MAX_V];

    for (int i = 0; i < MAX_V; ++i)
    {
      for (int j = 0; j < MAX_V; ++j)
      {
        double a = map.GetValue(i, j);
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
		IupSetAttribute(m_tf_plot, "AXS_ZMIN", "-1.001");
		IupSetAttribute(m_tf_plot, "AXS_ZAUTOMAX", "NO");
		IupSetAttribute(m_tf_plot, "AXS_ZMAX", "1.001");
		IupSetAttribute(m_tf_plot, "AXS_XLABEL", "Scalar Value");
		IupSetAttribute(m_tf_plot, "AXS_YLABEL", "Gradient");
		IupSetAttribute(m_tf_plot, "AXS_ZLABEL", "Alpha");
    //IupSetAttribute(m_tf_plot, "COLORSCHEME", "kw");
		//IupSetAttribute(m_tf_plot, "ROTATE", "0:0:-90");
		IupSetAttribute(m_tf_plot, "REDRAW", NULL);

		delete[] data;

		return true;
	}

	bool TransferFunction2D::Generate()
	{
		// TO COPY FROM 1D
		return true;
	}
}