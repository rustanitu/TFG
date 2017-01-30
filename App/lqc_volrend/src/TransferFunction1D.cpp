#include <volrend/TransferFunction1D.h>

#include <fstream>
#include <cstdlib>
#include <iup_plot.h>

namespace vr
{
	TransferFunction1D::TransferFunction1D (double v0, double v1)
		: m_v0(v0), m_v1(v1), m_values_size(0), m_built(false), m_interpolation_type(TFInterpolationType::LINEAR)
		, m_indexes(NULL), m_values(NULL), m_center(NULL), m_boundary(0), m_thickness(1)
		, m_direct_tf(false), m_gordon_tf(false), m_peakbased_tf(false), m_gaussian_bx(true)
	{
		m_cpt_rgb.clear ();
		m_cpt_alpha.clear ();
		m_transferfunction = NULL;
		m_gradients = NULL;
		printf("TransferFunction1D criado.\n");
	}

	TransferFunction1D::~TransferFunction1D ()
	{
		printf("TransferFunction1D destruido.\n");
		m_cpt_rgb.clear ();
		m_cpt_alpha.clear ();
		delete[] m_transferfunction;
		delete[] m_gradients;

		delete[] m_indexes;
		delete[] m_values;
		delete[] m_center;
	}

	const char* TransferFunction1D::GetNameClass ()
	{
		return "TrasnferFunction1D";
	}

	void TransferFunction1D::AddRGBControlPoint (TransferControlPoint rgb)
	{
		m_cpt_rgb.push_back (rgb);
	}

	void TransferFunction1D::AddAlphaControlPoint (TransferControlPoint alpha)
	{
		m_cpt_alpha.push_back (alpha);
	}

	void TransferFunction1D::ClearControlPoints ()
	{
		m_cpt_rgb.clear ();
		m_cpt_alpha.clear ();
	}

	void TransferFunction1D::ClearAlphaControlPoints()
	{
		m_cpt_alpha.clear();
		m_built = false;
	}

	gl::GLTexture1D* TransferFunction1D::GenerateTexture_RGBA ()
	{
		printf("TransferFunction1D: GenerateTexture_RGBA.\n");

		if (!m_built)
			Build (m_interpolation_type);

		if (m_transferfunction)
		{
			gl::GLTexture1D* ret = new gl::GLTexture1D (m_tflenght);
			ret->GenerateTexture (GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_BORDER);
			float* data = new float[m_tflenght * 4];
			for (int i = 0; i < m_tflenght; i++)
			{
				data[(i * 4)] = (float)(m_transferfunction[i].x);
				data[(i * 4) + 1] = (float)(m_transferfunction[i].y);
				data[(i * 4) + 2] = (float)(m_transferfunction[i].z);
				data[(i * 4) + 3] = (float)(m_transferfunction[i].w);
			}
			ret->SetData ((void*)data, GL_RGBA32F, GL_RGBA, GL_FLOAT);
			delete[] data;
			return ret;
		}
		return NULL;
	}

	void TransferFunction1D::Build (TFInterpolationType type)
	{
		if (m_transferfunction)
			delete[] m_transferfunction;
		m_transferfunction = new lqc::Vector4d[256];

		if (type == TFInterpolationType::LINEAR)
		{
			int numTF = 0;
			for (int i = 0; i < (int)m_cpt_rgb.size () - 1; i++)
			{
				int steps = m_cpt_rgb[i + 1].m_isoValue - m_cpt_rgb[i].m_isoValue;
				for (int j = 0; j < steps; j++)
				{
					float k = (float)j / (float)(steps);
					lqc::Vector4d diff = m_cpt_rgb[i + 1].m_color - m_cpt_rgb[i].m_color;
					diff = diff * k;
					m_transferfunction[numTF++] = m_cpt_rgb[i].m_color + diff;
				}
			}
			m_tflenght = numTF;

			numTF = 0;
			for (int i = 0; i < (int)m_cpt_alpha.size () - 1; i++)
			{
				int steps = m_cpt_alpha[i + 1].m_isoValue - m_cpt_alpha[i].m_isoValue;
				for (int j = 0; j < steps; j++)
				{
					float k = (float)j / (float)(steps);
					lqc::Vector4d diff = m_cpt_alpha[i + 1].m_color - m_cpt_alpha[i].m_color;
					diff = diff * k;
					m_transferfunction[numTF++].w = m_cpt_alpha[i].m_color.w + diff.w;
				}
			}
		}
		else if (type == TFInterpolationType::CUBIC)
		{
			std::vector<TransferControlPoint> tempColorKnots = m_cpt_rgb;
			std::vector<TransferControlPoint> tempAlphaKnots = m_cpt_alpha;

			Cubic* colorCubic = Cubic::CalculateCubicSpline ((int)m_cpt_rgb.size () - 1, tempColorKnots);
			Cubic* alphaCubic = Cubic::CalculateCubicSpline ((int)m_cpt_alpha.size () - 1, tempAlphaKnots);

			int numTF = 0;
			for (int i = 0; i < (int)m_cpt_rgb.size () - 1; i++)
			{
				int steps = m_cpt_rgb[i + 1].m_isoValue - m_cpt_rgb[i].m_isoValue;
				for (int j = 0; j < steps; j++)
				{
					float k = (float)j / (float)(steps);
					m_transferfunction[numTF++] = colorCubic[i].GetPointOnSpline (k);
				}
			}
			m_tflenght = numTF;

			numTF = 0;
			for (int i = 0; i < (int)m_cpt_alpha.size () - 1; i++)
			{
				int steps = m_cpt_alpha[i + 1].m_isoValue - m_cpt_alpha[i].m_isoValue;
				for (int j = 0; j < steps; j++)
				{
					float k = (float)j / (float)(steps);
					m_transferfunction[numTF++].w = alphaCubic[i].GetPointOnSpline (k).w;
				}
			}
		}
		printf ("TransferFunction1D: Build!\n");
		m_built = true;
	}

	lqc::Vector4d TransferFunction1D::Get (double value)
	{
		if (!m_built)
			Build (m_interpolation_type);

		lqc::Vector4d vf = lqc::Vector4d (0);
		if (!(value >= 0.0 && value <= 255.0))
			return lqc::Vector4d (0);

		if (value == (float)(m_tflenght - 1))
		{
			return m_transferfunction[m_tflenght - 1];
		}
		else
		{
			lqc::Vector4d v1 = m_transferfunction[(int)value];
			lqc::Vector4d v2 = m_transferfunction[((int)value) + 1];

			double t = value - (int)value;

			vf = (1.0 - t)*v1 + (t)*v2;
		}

		return vf;
	}

	void TransferFunction1D::PrintControlPoints ()
	{
		printf ("Print Transfer Function: Control Points\n");
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
		printf ("\n");
	}

	void TransferFunction1D::PrintTransferFunction ()
	{
		printf ("Print Transfer Function: Control Points\n");
		printf ("  Format: \"IsoValue: Red Green Blue, Alpha\"\n");
		for (int i = 0; i < m_tflenght; i++)
		{
			printf ("%d: %.2f %.2f %.2f, %.2f\n", i, m_transferfunction[i].x
				, m_transferfunction[i].y, m_transferfunction[i].z, m_transferfunction[i].w);
		}
	}

	/*
	void TransferFunction1D::Save (char* filename, TFFormatType format)
	{
	std::string filesaved;
	filesaved.append (RESOURCE_LIBLQC_PATH);
	filesaved.append ("TransferFunctions/");
	filesaved.append (filename);
	if (format == TFFormatType::LQC)
	{
	filesaved.append (".tf1d");
	std::ofstream myfile (filesaved.c_str ());
	if (myfile.is_open ())
	{
	myfile << 0 << "\n";
	myfile << (int)m_cpt_rgb.size () << "\n";
	for (int i = 0; i < (int)m_cpt_rgb.size (); i++)
	{
	myfile << m_cpt_rgb[i].m_color.x << " " <<
	m_cpt_rgb[i].m_color.y << " " <<
	m_cpt_rgb[i].m_color.z << " " <<
	m_cpt_rgb[i].m_isoValue << " " << "\n";
	}
	myfile << (int)m_cpt_alpha.size () << "\n";
	for (int i = 0; i < (int)m_cpt_alpha.size (); i++)
	{
	myfile << m_cpt_alpha[i].m_color.w << " " <<
	m_cpt_alpha[i].m_isoValue << " " << "\n";
	}
	myfile.close ();
	printf ("lqc: Transfer Function 1D Control Points Saved!\n");
	}
	else
	{
	printf ("lqc: Error on opening file at VRTransferFunction::Save().\n");
	}
	}
	}

	bool TransferFunction1D::Load (std::string filename, TFFormatType format)
	{
	std::string filesaved;
	filesaved.append (RESOURCE_LIBLQC_PATH);
	filesaved.append ("TransferFunctions/");
	filesaved.append (filename);
	if (format == TFFormatType::LQC)
	{
	filesaved.append (".tf1d");
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
	m_cpt_rgb.push_back (TransferControlPoint (r, g, b, isovalue));
	}

	int cpt_alpha_size;
	myfile >> cpt_alpha_size;
	for (int i = 0; i < cpt_alpha_size; i++)
	{
	myfile >> a >> isovalue;
	m_cpt_alpha.push_back (TransferControlPoint (a, isovalue));
	}
	myfile.close ();
	printf ("lqc: Transfer Function 1D Control Points Loaded!\n");
	return true;
	}
	else
	printf ("lqc: Error on opening file at VRTransferFunction::AddControlPointsReadFile().\n");
	}
	return false;
	}*/

	double TransferFunction1D::CenteredTriangleFunction(double max, double base, double center, const int& v)
	{
		//  boundary center
		//         .
		//        / \       |
		//       / | \      |
		//      /  |  \     |
		//     /   |   \    | top
		//    /    |    \   |
		//   /     |     \  |
		//  /      |      \ |
		//  ---- center ++++ 
		// |-------|-------|
		//       base

		double top = 1.0f;// / sqrt(PI * 2 * base * base / 9);
		double a = 0.0f;
		double x = m_values[v];
		if (x >= -base && x <= base)
		{
			if (x >= center && center < base)
			{
				a = -(top * x) / (base - center);
				a += (top * base) / (base - center);
			}
			else
			{
				a = (top * x) / (base + center);
				a += (top * base) / (base + center);
			}
		}

		return fmin(a, max);
	}

	double TransferFunction1D::CenteredGaussianFunction(double max, double base, double u, const int& v)
	{
		double sigma = base / 3.0f;
		double two_sigma_quad = 2 * sigma * sigma;
		double x = m_values[v];
		double g = exp(-(x - u)*(x - u) / two_sigma_quad);// / sqrt(PI * two_sigma_quad);
		return fmin(g, max);
	}

	/// <summary>
	/// Generates a transfer function file at a given path.
	/// If a file with the same path already exists, it'll
	/// be replaced.
	/// </summary>
	/// <returns>Returns true if the transfer function can
	/// be generated, false otherwise.</returns>
	bool TransferFunction1D::GenerateGordonBased()
	{
		if ( !m_indexes || !m_values )
			throw std::exception_ptr();

		if (m_direct_tf || m_peakbased_tf)
			throw std::domain_error("Transfer function already set (not a Gordon's one).");

		printf("TransferFunction1D: GenerateGordonBased (from ATFG).\n");

		ClearAlphaControlPoints();
		IupSetAttribute(m_tf_plot, "CLEAR", "YES");
		IupPlotBegin(m_tf_plot, 0);

		double amax = 1.0f;

		double last_a = 0.0f;
		int b = 0;
		bool finished = true;

		IupPlotAdd(m_tf_plot, 0, 1.0f);

		// Assign opacity to transfer function
		for ( int i = 0; i < m_values_size; ++i )
		{
			int value = m_indexes[i];
			double x = m_values[value];

			double a = 0.0f;
			if (m_gaussian_bx)
				a = CenteredGaussianFunction(amax, 1.0f / m_thickness, m_center[value], value);
			else
				a = CenteredTriangleFunction(amax, 1.0f / m_thickness, m_center[value], value);

			if ( m_boundary != 0 )
			{
				if ( last_a <= 0.1f && a > 0.1f )
				{
					++b;
					finished = false;
					if ( b == m_boundary && i - 1 >= 0 )
					{
						AddAlphaControlPoint(TransferControlPoint(0.0f, m_indexes[i - 1]));
						IupPlotAdd(m_tf_plot, value - 1, 0.0f);
					}
				}
				if ( a <= 0.1f )
				{
					finished = true;
					AddAlphaControlPoint(TransferControlPoint(0.0f, value));
					IupPlotAdd(m_tf_plot, value, 0.0f);
				}
				last_a = a;
				if ( b != m_boundary || finished )
					continue;
			}

			AddAlphaControlPoint(TransferControlPoint(a, value));
			IupPlotAdd(m_tf_plot, value, a);
		}

		IupPlotAdd(m_tf_plot, 255, 1.0f);

		IupPlotEnd(m_tf_plot);
		IupSetAttribute(m_tf_plot, "DS_NAME", "Transfer Function");
		IupSetAttribute(m_tf_plot, "DS_COLOR", "128 128 128");
		IupSetAttribute(m_tf_plot, "REDRAW", "YES");

		return true;
	}

	bool TransferFunction1D::Generate()
	{
		if (!m_indexes || !m_values)
			throw std::exception_ptr();

		if (m_gordon_tf)
			throw std::domain_error("Transfer function already set as Gordon's.");

		printf("TransferFunction1D: Generate (from ATFG).\n");

		ClearAlphaControlPoints();
		IupSetAttribute(m_tf_plot, "CLEAR", "YES");
		IupPlotBegin(m_tf_plot, 0);

		double amax = 0.0f;
		for (int i = 0; i < m_values_size; ++i) {
			amax = fmax(amax, m_values[i]);
		}

		double last_a = 0.0f;
		int b = 0;
		bool finished = true;
		double base = m_thickness;

		// Assign opacity to transfer function
		for (int i = 0; i < m_values_size; ++i)
		{
			int value = m_indexes[i];
			double a = fmin(m_values[i] * m_thickness / amax, 1.0f);

			if (m_boundary != 0) {
				if (last_a <= 0.1f && a > 0.1f) {
					++b;
					finished = false;
					if (b == m_boundary && i - 1 >= 0) {
						AddAlphaControlPoint(TransferControlPoint(0.0f, m_indexes[i - 1]));
						IupPlotAdd(m_tf_plot, m_indexes[i - 1], 0.0f);
					}
				}
				if (a <= 0.1f) {
					finished = true;
					AddAlphaControlPoint(TransferControlPoint(0.0f, value));
					IupPlotAdd(m_tf_plot, value, 0.0f);
				}
				last_a = a;
				if (b != m_boundary || finished)
					continue;
			}

			AddAlphaControlPoint(TransferControlPoint(a, value));
			IupPlotAdd(m_tf_plot, value, a);
		}

		IupPlotEnd(m_tf_plot);
		IupSetAttribute(m_tf_plot, "DS_NAME", "Transfer Function");
		IupSetAttribute(m_tf_plot, "DS_COLOR", "128 128 128");
		IupSetAttribute(m_tf_plot, "REDRAW", "YES");

		return true;
	}

	/// <summary>
	/// Specifies the distance between a intensity value
	/// and its closest boundary. Thus, the input arrays'
	/// size must range from 2 to 256. Any array content
	/// whose index is greater than 255, it'll be ignored.
	/// </summary>
	/// <param name="values">The values array.</param>
	/// <param name="distances">The distances to the closest boundaries.</param>
	/// <param name="sigmas">The sigmas of the boundaries.</param>
	/// <param name="n">The input arrays' size.</param>
	void TransferFunction1D::SetClosestBoundaryDistances(int* values, double* distances, double* h, const int& n)
	{
		if (n < 2 || n > MAX_V)
			throw std::length_error("At least 2 values are needed to interpolate the transfer function!");

		if (m_direct_tf || m_peakbased_tf)
			throw std::domain_error("Transfer function already set (not as Gordon).");

		m_gordon_tf = true;
		m_values_size = n;

		delete[] m_indexes;
		delete[] m_values;
		delete[] m_center;

		if (values)
			m_indexes = values;

		if (distances)
			m_values = distances;

		if (h)
			m_center = h;
	}

	void TransferFunction1D::SetAlphaValues(int* values, double* alphas, const int& n)
	{
		if ( n < 2 || n > MAX_V )
			throw std::length_error("At least 2 values are needed to interpolate the transfer function!");

		if (m_gordon_tf || m_peakbased_tf)
			throw std::domain_error("Transfer function already set Direct.");

		m_direct_tf = true;
		m_values_size = n;

		delete[] m_indexes;
		delete[] m_values;

		if ( values )
			m_indexes = values;

		if ( alphas )
			m_values = alphas;
	}

	void TransferFunction1D::SetPeakPoints(int* peaks, double* values, const int& n)
	{
		if (n < 2 || n > MAX_V)
			throw std::length_error("At least 2 values are needed to interpolate the transfer function!");

		if (m_gordon_tf || m_direct_tf)
			throw std::domain_error("Transfer function already set as Gordon's.");

		m_peakbased_tf = true;
		m_values_size = n;

		delete[] m_indexes;
		delete[] m_values;

		if (peaks)
			m_indexes = peaks;

		if (values)
			m_values = values;
	}
}