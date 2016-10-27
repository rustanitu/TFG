#include <volrend/TransferFunction1D.h>

#include <fstream>
#include <cstdlib>
#include <iup_plot.h>

namespace vr
{
  TransferFunction1D::TransferFunction1D (double v0, double v1)
    : m_v0(v0), m_v1(v1), m_values_size(0), m_built(false), m_interpolation_type(TFInterpolationType::LINEAR)
    , m_value(NULL), m_distance(NULL), m_boundary(0), m_thickness(1)
  {
    m_cpt_rgb.clear ();
    m_cpt_alpha.clear ();
    m_transferfunction = NULL;
  }

  TransferFunction1D::~TransferFunction1D ()
  {
    m_cpt_rgb.clear ();
    m_cpt_alpha.clear ();
    if (m_transferfunction)
      m_transferfunction;
    if (m_gradients)
      m_gradients;

    delete[] m_value;
    delete[] m_distance;
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

  gl::GLTexture1D* TransferFunction1D::GenerateTexture_1D_RGBA ()
  {
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
    printf ("lqc: Transfer Function 1D Built!\n");
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

  float TransferFunction1D::CenteredTriangleFunction(float max, float base, unsigned char v)
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
    //  ------ 0 ++++++ 
    // |-------|-------|
    //       base

    float a = 0.0f;
    float x = m_distance[v];
    if (x >= -base && x <= base)
    {
      if (x >= 0.0)
        a = -(max * x) / base;
      else
        a = (max * x) / base;

      a += max;
      m_valid[v] = true;
    }

    return a;
  }

  bool TransferFunction1D::ValidValue(int v)
  {
    return m_valid[v];
  }

  /// <summary>
  /// Generates a transfer function file at a given path.
  /// If a file with the same path already exists, it'll
  /// be replaced.
  /// </summary>
  /// <returns>Returns true if the transfer function can
  /// be generated, false otherwise.</returns>
  bool TransferFunction1D::Generate()
  {
    if (!m_value || !m_distance)
      throw std::exception_ptr();

    ClearAlphaControlPoints();
    IupSetAttribute(m_tf_plot, "CLEAR", "YES");
    IupPlotBegin(m_tf_plot, 0);
    IupSetAttribute(m_bx_plot, "CLEAR", "YES");
    IupPlotBegin(m_bx_plot, 0);

    for (int i = 0; i < 256; ++i)
      m_valid[i] = false;

    float amax = 1.0f;
    float base = m_thickness;

    // Assign opacity to transfer function
    for (int i = 0; i < m_values_size; ++i)
    {
      unsigned char value = m_value[i];
      float x = m_distance[value];

      IupPlotAdd(m_bx_plot, value, fmax(fmin(x, 20), -20));
      double a = CenteredTriangleFunction(amax, base, value);

      AddAlphaControlPoint(TransferControlPoint(a, value));
      IupPlotAdd(m_tf_plot, value, a);
    }

    IupPlotEnd(m_tf_plot);
    IupSetAttribute(m_tf_plot, "DS_NAME", "Transfer Function");
    IupSetAttribute(m_tf_plot, "DS_COLOR", "128 128 128");
    IupSetAttribute(m_tf_plot, "REDRAW", "YES");

    IupPlotEnd(m_bx_plot);
    IupSetAttribute(m_bx_plot, "DS_NAME", "p(v)");

    IupPlotBegin(m_bx_plot, 0);
    IupPlotAdd(m_bx_plot, 0, 0);
    IupPlotAdd(m_bx_plot, 255, 0);
    IupPlotEnd(m_bx_plot);
    IupSetAttribute(m_bx_plot, "DS_NAME", "0");
    IupSetAttribute(m_bx_plot, "DS_COLOR", "0 0 0");

    IupPlotBegin(m_bx_plot, 0);
    IupPlotAdd(m_bx_plot, 0, m_thickness);
    IupPlotAdd(m_bx_plot, 255, m_thickness);
    IupPlotEnd(m_bx_plot);
    IupSetAttribute(m_bx_plot, "DS_NAME", "b(x)");
    IupSetAttribute(m_bx_plot, "DS_COLOR", "128 128 0");

    IupPlotBegin(m_bx_plot, 0);
    IupPlotAdd(m_bx_plot, 0, -m_thickness);
    IupPlotAdd(m_bx_plot, 255, -m_thickness);
    IupPlotEnd(m_bx_plot);
    IupSetAttribute(m_bx_plot, "DS_NAME", "b(x)");
    IupSetAttribute(m_bx_plot, "DS_COLOR", "128 128 0");

    IupSetAttribute(m_bx_plot, "REDRAW", "YES");

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
  void TransferFunction1D::SetClosestBoundaryDistances(unsigned char* values, float* distances, int n)
  {
    if (n < 2 || n > MAX_V)
      throw std::length_error("At least 2 values are needed to interpolate the transfer funciton!");

    m_values_size = n;

    delete[] m_value;
    delete[] m_distance;

    if (values)
      m_value = values;

    if (distances)
      m_distance = distances;
  }

}