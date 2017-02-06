#include <volrend/TransferFunction2D.h>

#include <fstream>
#include <cstdlib>
#include <iup_mglplot.h>

namespace vr
{
	TransferFunction2D::TransferFunction2D (double v0, double v1)
		: m_v0(v0), m_v1(v1), m_built(false), m_interpolation_type(TFInterpolationType::LINEAR)
    , m_distances(NULL), m_sigma(0.0f)
	{
		printf("TransferFunction2D criado.\n");
		m_width = m_height = MAX_V;
		ClearRGBControlPoints();
	}

	TransferFunction2D::~TransferFunction2D ()
	{
		printf("TransferFunction2D destruido.\n");
    if (m_distances) {
      for (int i = 0; i < m_width; ++i)
        delete[] m_distances[i];
      delete[] m_distances;
    }
	}

	const char* TransferFunction2D::GetNameClass ()
	{
		return "TrasnferFunction2D";
	}

	void TransferFunction2D::AddRGBControlPoint(lqc::Vector3f rgb, int v, int g)
	{ 
		m_transferfunction[v][g].x = rgb.x;
		m_transferfunction[v][g].y = rgb.y;
		m_transferfunction[v][g].z = rgb.z;
	}

	void TransferFunction2D::AddAlphaControlPoint(double alpha, int v, int g)
	{
		m_transferfunction[v][g].w = alpha;
	}

	void TransferFunction2D::ClearRGBControlPoints ()
	{
		for (int v = 0; v < m_height; ++v)
		{
			for (int g = 0; g < m_width; ++g)
			{
				m_transferfunction[v][g].x = -DBL_MAX;
				m_transferfunction[v][g].y = -DBL_MAX;
				m_transferfunction[v][g].z = -DBL_MAX;
			}
		}

    m_built = false;
	}

	void TransferFunction2D::ClearAlphaControlPoints()
	{
		for (int v = 0; v < m_height; ++v) {
			for (int g = 0; g < m_width; ++g) {
				m_transferfunction[v][g].w = -DBL_MAX;
			}
		}

    m_built = false;
	}

	gl::GLTexture2D* TransferFunction2D::GenerateTexture_RGBA ()
	{
		printf("TransferFunction2D: GenerateTexture_RGBA.\n");

		if (!m_built)
			Build (m_interpolation_type);

		if (m_transferfunction)
		{
			gl::GLTexture2D* ret = new gl::GLTexture2D (m_width, m_height);
			ret->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
			float* data = new float[m_width * m_height * 4];
			int c = 0;
			for (int v = 0; v < m_height; v++)
			{
				for (int g = 0; g < m_width; g++)
				{
					data[c++] = (float)(m_transferfunction[v][g].x);
					data[c++] = (float)(m_transferfunction[v][g].y);
					data[c++] = (float)(m_transferfunction[v][g].z);
					data[c++] = (float)(m_transferfunction[v][g].w);
				}
			}
			ret->SetData ((void*)data, GL_RGBA32F, GL_RGBA, GL_FLOAT);
			delete[] data;
			return ret;
		}
		return NULL;
	}

	void TransferFunction2D::Build (TFInterpolationType type)
	{
		double gauss[3][3] = {
			{ 1.0f, 2.0f, 1.0f },
			{ 2.0f, 4.0f, 2.0f },
			{ 1.0f, 2.0f, 1.0f },
		};

    for (int v = 0; v < m_width; ++v)
		{
      for (int g = 0; g < m_height; ++g)
			{
				if (m_transferfunction[v][g].x == -DBL_MAX)
				{
					lqc::Vector4d average = lqc::Vector4d::Zero();
					float w = 0.0f;
					for (int i = 0; i < 3; i++) {
						for (int j = 0; j < 3; j++) {
							if (v + i - 1 >= 0 && v + i - 1 < m_width &&
								g + j - 1 >= 0 && g + j - 1 < m_height &&
								m_transferfunction[v + i - 1][g + j - 1].x != -DBL_MAX) {
								average += gauss[i][j] * m_transferfunction[v + i - 1][g + j - 1];
								w += gauss[i][j];
							}
						}
					}
					if (w > 0.0f)
					{
						m_transferfunction[v][g].x = average.x / w;
						m_transferfunction[v][g].y = average.y / w;
						m_transferfunction[v][g].z = average.z / w;
					}
				}

				if (m_transferfunction[v][g].w == -DBL_MAX)
				{
					m_transferfunction[v][g].w = 0.0f;
					float average = 0.0f;
					float w = 0.0f;
					for (int i = 0; i < 3; i++)
          {
						for (int j = 0; j < 3; j++)
            {
							if (v + i - 1 >= 0 && v + i - 1 < m_width && g + j - 1 >= 0 && g + j - 1 < m_height)
              {
                if (m_transferfunction[v + i - 1][g + j - 1].w != -DBL_MAX)
								  average += gauss[i][j] * m_transferfunction[v + i - 1][g + j - 1].w;
								w += gauss[i][j];
							}
						}
					}
					if (w > 0.0f)
						m_transferfunction[v][g].w = average / w;
				}
			}
		}

		printf ("TransferFunction2D: Build!\n");
		m_built = true;
	}

	lqc::Vector4d TransferFunction2D::Get (double value)
	{
		if (!m_built)
			Build (m_interpolation_type);

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
		for (int i = 0; i < m_width; i++)
		{
			for (int j = 0; j < m_width; j++) {
				printf("[%d][%d]: %.2f %.2f %.2f, %.2f\n", i, j, m_transferfunction[i][j].x
					, m_transferfunction[i][j].y, m_transferfunction[i][j].z, m_transferfunction[i][j].w);
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

	double TransferFunction2D::CenteredTriangleFunction(double max, double base, double center, const int& v, const int& g)
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
		double x = m_distances[v][g];
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

	double TransferFunction2D::CenteredGaussianFunction(double max, double base, double u, const int& v, const int& g)
	{
		double sigma = base / 3.0f;
    double x = m_distances[v][g];
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

		IupSetAttribute(m_tf_plot, "CLEAR", "YES");

		int index = IupMglPlotNewDataSet(m_tf_plot, 1);

		ClearAlphaControlPoints();

		double* data = new double[MAX_V*MAX_V];

		double amax = 50.0f;
		// Assign opacity to transfer function
		for ( int i = 0; i < MAX_V; ++i )
		{
			for (int j = 0; j < MAX_V; ++j)
			{
				double x = m_distances[i][j];
        if (x == -DBL_MAX)
          continue;

				double a = 0.0f;
        if (m_gaussian_bx)
          a = CenteredGaussianFunction(amax, 1.0f / m_thickness, m_sigma, i, j);
        else
          a = CenteredTriangleFunction(amax, 1.0f / m_thickness, m_sigma, i, j);

				AddAlphaControlPoint(a, i, j);
			}
		}

    Build(LINEAR);

    for (int i = 0; i < MAX_V; ++i) {
      for (int j = 0; j < MAX_V; ++j) {
        data[i + MAX_V*j] = m_transferfunction[i][j].w;
      }
    }

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
    IupSetAttribute(m_tf_plot, "COLORSCHEME", "kw");
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

	/// <summary>
	/// Specifies the distance between a intensity value
	/// and its closest boundary. Thus, the input arrays'
	/// size must range from 2 to MAX_V. Any array content
	/// whose index is greater than 255, it'll be ignored.
	/// </summary>
	/// <param name="values">The values array.</param>
	/// <param name="distances">The distances to the closest boundaries.</param>
	/// <param name="sigmas">The sigmas of the boundaries.</param>
	/// <param name="n">The input arrays' size.</param>
	void TransferFunction2D::SetClosestBoundaryDistances(double** distances)
	{
		if (m_distances) {
			for (int i = 0; i < m_width; ++i)
				delete[] m_distances[i];
			delete[] m_distances;
		}

		m_width = m_height = MAX_V;

		if (distances)
			m_distances = distances;
	}
}