#include <volrend/TransferFunctionGersa.h>

#include <fstream>
#include <cstdlib>

namespace vr
{
  TransferFunctionGersa::TransferFunctionGersa (double v0, double v1)
    : m_v0 (v0), m_v1 (v1), m_built (false), m_interpolation_type (TFInterpolationType::LINEAR)
  {
    m_cpt_rgb.clear ();
    m_cpt_alpha.clear ();
    m_transferfunction = NULL;
  }

  TransferFunctionGersa::~TransferFunctionGersa ()
  {
    m_cpt_rgb.clear ();
    m_cpt_alpha.clear ();
    if (m_transferfunction)
      m_transferfunction;
    if (m_gradients)
      m_gradients;
  }

  const char* TransferFunctionGersa::GetNameClass ()
  {
    return "TrasnferFunctionGersa";
  }

  void TransferFunctionGersa::AddRGBControlPoint (TransferControlPoint rgb)
  {
    m_cpt_rgb.push_back (rgb);
  }

  void TransferFunctionGersa::AddAlphaControlPoint (TransferControlPoint alpha)
  {
    m_cpt_alpha.push_back (alpha);
  }

  void TransferFunctionGersa::ClearControlPoints ()
  {
    m_cpt_rgb.clear ();
    m_cpt_alpha.clear ();
  }

  gl::GLTexture1D* TransferFunctionGersa::GenerateTexture_1D_RGBA ()
  {
    if (!m_built)
      Build (m_interpolation_type);

    if (m_transferfunction)
    {
      gl::GLTexture1D* ret = new gl::GLTexture1D (m_tflenght);
      ret->GenerateTexture (GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
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

  void TransferFunctionGersa::Build (TFInterpolationType type)
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

  lqc::Vector4d TransferFunctionGersa::Get (double value)
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

  void TransferFunctionGersa::PrintControlPoints ()
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

  void TransferFunctionGersa::PrintTransferFunction ()
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
  void TransferFunctionGersa::Save (char* filename, TFFormatType format)
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

  bool TransferFunctionGersa::Load (std::string filename, TFFormatType format)
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

}