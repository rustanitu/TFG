#include <volrend/TransferFunction.h>

#include <lqc/lqcdefines.h>

#include <fstream>

#include <cstdlib>

namespace vr
{
  /*TransferFunction1D::TransferFunction1D (double v0, double v1)
    : m_v0 (v0), m_v1 (v1), m_built (false)
  {
    m_cpt_rgb.clear();
    m_cpt_alpha.clear();
    m_transferfunction = NULL;
  }

  TransferFunction1D::~TransferFunction1D ()
  {
    m_cpt_rgb.clear();
    m_cpt_alpha.clear();
    if (m_transferfunction)
      m_transferfunction;
    if (m_gradients)
      m_gradients;
  }

  const char* TransferFunction1D::GetNameClass ()
  {
    return "TrasnferFunction1D";
  }

  void TransferFunction1D::AddRGBControlPoint (TransferControlPoint rgb)
  {
    m_cpt_rgb.push_back(rgb);
  }

  void TransferFunction1D::AddAlphaControlPoint (TransferControlPoint alpha)
  {
    m_cpt_alpha.push_back(alpha);
  }

  void TransferFunction1D::ClearControlPoints ()
  {
    m_cpt_rgb.clear();
    m_cpt_alpha.clear();
  }

  GLTexture1D* TransferFunction1D::GenerateTexture_1D_RGBA ()
  {
    if (!m_built)
      Build (TFInterpolationType::LINEAR);

    if (m_transferfunction)
    {
      GLTexture1D* ret = new GLTexture1D (m_tflenght);
      ret->GenerateTexture (GL_LINEAR, GL_LINEAR, GL_CLAMP);
      float* data = new float[m_tflenght * 4];
      for (int i = 0; i < m_tflenght; i++)
      {
        data[(i * 4)] = (float)(m_transferfunction[i].x);
        data[(i * 4) + 1] = (float)(m_transferfunction[i].y);
        data[(i * 4) + 2] = (float)(m_transferfunction[i].z);
        data[(i * 4) + 3] = (float)(m_transferfunction[i].w);
      }
      ret->SetData ((void*)data, GL_RGBA, GL_RGBA, GL_FLOAT);
      delete[] data;
      return ret;
    }
    return NULL;
  }

  void TransferFunction1D::Build (TFInterpolationType type)
  {
    if (m_transferfunction)
      delete[] m_transferfunction;
    m_transferfunction = new Vector4f[256];

    if (type == TFInterpolationType::LINEAR)
    {
      int numTF = 0;
      for (int i = 0; i < (int)m_cpt_rgb.size() - 1; i++)
      {
        int steps = m_cpt_rgb[i+1].m_isoValue - m_cpt_rgb[i].m_isoValue;
        for (int j = 0; j < steps; j++)
        {
          float k = (float)j / (float)(steps);
          Vector4f diff = m_cpt_rgb[i + 1].m_color - m_cpt_rgb[i].m_color;
          diff = diff * k;
          m_transferfunction[numTF++] = m_cpt_rgb[i].m_color + diff;
        }
      }
      m_tflenght = numTF;

      numTF = 0;
      for (int i = 0; i < (int)m_cpt_alpha.size() - 1; i++)
      {
        int steps = m_cpt_alpha[i+1].m_isoValue - m_cpt_alpha[i].m_isoValue;
        for (int j = 0; j < steps; j++)
        {
          float k = (float)j / (float)(steps);
          Vector4f diff = m_cpt_alpha[i + 1].m_color - m_cpt_alpha[i].m_color;
          diff = diff * k;
          m_transferfunction[numTF++].w = m_cpt_alpha[i].m_color.w + diff.w;
        }
      }
    }
    else if (type == TFInterpolationType::CUBIC)
    {
      std::vector<TransferControlPoint> tempColorKnots = m_cpt_rgb;
      std::vector<TransferControlPoint> tempAlphaKnots = m_cpt_alpha;

      Cubic* colorCubic = Cubic::CalculateCubicSpline((int)m_cpt_rgb.size()-1, tempColorKnots);
      Cubic* alphaCubic = Cubic::CalculateCubicSpline((int)m_cpt_alpha.size()-1, tempAlphaKnots);

      int numTF = 0;
      for (int i = 0; i < (int)m_cpt_rgb.size() - 1; i++)
      {
        int steps = m_cpt_rgb[i+1].m_isoValue - m_cpt_rgb[i].m_isoValue;
        for (int j = 0; j < steps; j++)
        {
          float k = (float)j / (float)(steps);
          m_transferfunction[numTF++] = colorCubic[i].GetPointOnSpline(k);
        }
      }
      m_tflenght = numTF;

      numTF = 0;
      for (int i = 0; i < (int)m_cpt_alpha.size() - 1; i++)
      {
        int steps = m_cpt_alpha[i+1].m_isoValue - m_cpt_alpha[i].m_isoValue;
        for (int j = 0; j < steps; j++)
        {
          float k = (float)j / (float)(steps);
          m_transferfunction[numTF++].w = alphaCubic[i].GetPointOnSpline(k).w;
        }
      }
    }
    printf("lqc: Transfer Function 1D Built!\n");
    m_built = true;
  }

  Vector4d TransferFunction1D::Get (double value)
  {
    if (!m_built)
      Build (TFInterpolationType::LINEAR);

    Vector4d vf = Vector4d(0);
    if (value >= (float)m_tflenght || value < 0.0f)
      return vf;

    if (value == (float)(m_tflenght - 1))
    {
      lqc::Vector4f v4f = m_transferfunction[m_tflenght - 1];
      return lqc::Vector4d (v4f.x, v4f.y, v4f.z, v4f.w);
    }
    else
    {
      Vector4f v4f1 = m_transferfunction[(int)value];
      Vector4d v1 = Vector4d (v4f1.x, v4f1.y, v4f1.z, v4f1.w);
      lqc::Vector4f v4f2 = m_transferfunction[((int)value) + 1];
      Vector4d v2 = Vector4d (v4f2.x, v4f2.y, v4f2.z, v4f2.w);

      double t = value - (int)value;

      vf = (1.0 - t)*v1 + (t)*v2;
    }

    return vf;
  }

  void TransferFunction1D::PrintControlPoints ()
  {
    printf("Print Transfer Function: Control Points\n");
    int rgb_pts = (int)m_cpt_rgb.size();
    printf("- Printing the RGB Control Points\n");
    printf("  Format: \"Number: Red Green Blue, Isovalue\"\n");
    for (int i = 0 ; i < rgb_pts ; i++)
    {
      printf("  %d: %.2f %.2f %.2f, %d\n", i+1, m_cpt_rgb[i].m_color.x, m_cpt_rgb[i].m_color.y, m_cpt_rgb[i].m_color.z, m_cpt_rgb[i].m_isoValue); 
    }
    printf("\n");

    int alpha_pts = (int)m_cpt_alpha.size();
    printf("- Printing the Alpha Control Points\n");
    printf("  Format: \"Number: Alpha, Isovalue\"\n");
    for (int i = 0 ; i < alpha_pts ; i++)
    {
      printf("  %d: %.2f, %d\n", i+1, m_cpt_alpha[i].m_color.w, m_cpt_alpha[i].m_isoValue); 
    }
    printf("\n");
  }

  void TransferFunction1D::PrintTransferFunction ()
  {
    printf("Print Transfer Function: Control Points\n");
    printf("  Format: \"IsoValue: Red Green Blue, Alpha\"\n");
    for (int i = 0; i < m_tflenght; i++)
    {
      printf("%d: %.2f %.2f %.2f, %.2f\n", i, m_transferfunction[i].x
        , m_transferfunction[i].y, m_transferfunction[i].z, m_transferfunction[i].w);
    }
  }

  void TransferFunction1D::Save (char* filename, TFFormatType format)
  {
    std::string filesaved;
    filesaved.append(RESOURCE_LIBLQC_PATH);
    filesaved.append("TransferFunctions/");
    filesaved.append(filename);
    if (format == TFFormatType::LQC)
    {
      filesaved.append(".tf1d");
      std::ofstream myfile (filesaved.c_str());
      if (myfile.is_open())
      {
        myfile << 0 << "\n";
        myfile << (int)m_cpt_rgb.size() << "\n";
        for(int i = 0; i < (int)m_cpt_rgb.size(); i++)
        {
          myfile << m_cpt_rgb[i].m_color.x << " " <<
            m_cpt_rgb[i].m_color.y << " " << 
            m_cpt_rgb[i].m_color.z << " " << 
            m_cpt_rgb[i].m_isoValue << " " << "\n";
        }
        myfile << (int)m_cpt_alpha.size() << "\n";
        for(int i = 0; i < (int)m_cpt_alpha.size(); i++)
        {
          myfile << m_cpt_alpha[i].m_color.w << " " <<
            m_cpt_alpha[i].m_isoValue << " " << "\n";
        }
        myfile.close();
        printf("lqc: Transfer Function 1D Control Points Saved!\n");
      }
      else
      {
        printf("lqc: Error on opening file at VRTransferFunction::Save().\n");
      }
    }
  }

  bool TransferFunction1D::Load (std::string filename, TFFormatType format)
  {
    std::string filesaved;
    filesaved.append(RESOURCE_LIBLQC_PATH);
    filesaved.append("TransferFunctions/");
    filesaved.append(filename);
    if (format == TFFormatType::LQC)
    {
      filesaved.append(".tf1d");
      std::ifstream myfile (filesaved.c_str());
      if (myfile.is_open())
      {
        int init;
        myfile >> init;

        int cpt_rgb_size;
        myfile >> cpt_rgb_size;
        float r,g,b,a;
        int isovalue;
        for(int i = 0; i < cpt_rgb_size; i++)
        {
          myfile >> r >> g >> b >> isovalue;
          m_cpt_rgb.push_back(TransferControlPoint(r,g,b,isovalue));
        }

        int cpt_alpha_size;
        myfile >> cpt_alpha_size;
        for(int i = 0; i < cpt_alpha_size; i++)
        {
          myfile >> a >> isovalue;
          m_cpt_alpha.push_back(TransferControlPoint(a,isovalue));
        }
        myfile.close();
        printf("lqc: Transfer Function 1D Control Points Loaded!\n");
        return true;
      }
      else
        printf("lqc: Error on opening file at VRTransferFunction::AddControlPointsReadFile().\n");
    }
    return false;
  }*/

TransferControlPoint::TransferControlPoint (double r, double g, double b, int isovalue)
  {
    m_color.x = r;
    m_color.y = g;
    m_color.z = b;
    m_color.w = 1.0f;
    m_isoValue = isovalue;
  }

TransferControlPoint::TransferControlPoint (double alpha, int isovalue)
  {
    m_color.x = 0.0f;
    m_color.y = 0.0f;
    m_color.z = 0.0f;
    m_color.w = alpha;
    m_isoValue = isovalue;
  }

  Cubic::Cubic ()
  {
  }

  Cubic::Cubic (lqc::Vector4d ain, lqc::Vector4d bin, lqc::Vector4d cin, lqc::Vector4d din)
    : a(ain), b(bin), c(cin), d(din)
  {
  }

  Cubic::~Cubic ()
  {
  }

  lqc::Vector4d Cubic::GetPointOnSpline (double s)
  {
    return (((d * s) + c) * s + b) * s + a;
  }

  Cubic* Cubic::CalculateCubicSpline (int n, std::vector<TransferControlPoint> v)
  {
    lqc::Vector4d* gamma = new lqc::Vector4d[n + 1];
    lqc::Vector4d* delta = new lqc::Vector4d[n + 1];
    lqc::Vector4d* D = new lqc::Vector4d[n + 1];
    int i;
    /* We need to solve the equation
    * taken from: http://mathworld.wolfram.com/CubicSpline.html
    [2 1       ] [D[0]]   [3(v[1] - v[0])  ]
    |1 4 1     | |D[1]|   |3(v[2] - v[0])  |
    |  1 4 1   | | .  | = |      .         |
    |    ..... | | .  |   |      .         |
    |     1 4 1| | .  |   |3(v[n] - v[n-2])|
    [       1 2] [D[n]]   [3(v[n] - v[n-1])]

    by converting the matrix to upper triangular.
    The D[i] are the derivatives at the control points.
    */

    //this builds the coefficients of the left matrix
    gamma[0] = lqc::Vector4d::Zero ();
    gamma[0].x = 1.0 / 2.0;
    gamma[0].y = 1.0 / 2.0;
    gamma[0].z = 1.0 / 2.0;
    gamma[0].w = 1.0 / 2.0;
    for (i = 1; i < n; i++)
    {
      gamma[i] = lqc::Vector4d::One () / ((4.0 * lqc::Vector4d::One ()) - gamma[i - 1]);
    }
    gamma[n] = lqc::Vector4d::One () / ((2.0 * lqc::Vector4d::One ()) - gamma[n - 1]);

    delta[0] = 3.0 * (v[1].m_color - v[0].m_color) * gamma[0];
    for (i = 1; i < n; i++)
    {
      delta[i] = (3.0 * (v[i + 1].m_color - v[i - 1].m_color) - delta[i - 1]) * gamma[i];
    }
    delta[n] = (3.0 * (v[n].m_color - v[n - 1].m_color) - delta[n - 1]) * gamma[n];

    D[n] = delta[n];
    for (i = n - 1; i >= 0; i--)
    {
      D[i] = delta[i] - gamma[i] * D[i + 1];
    }

    // now compute the coefficients of the cubics 
    Cubic* C = new Cubic[n];
    for (i = 0; i < n; i++)
    {
      C[i] = Cubic(v[i].m_color, D[i], 3.0 * (v[i + 1].m_color- v[i].m_color) - 2.0 * D[i] - D[i + 1],
        2.0 * (v[i].m_color - v[i + 1].m_color) + D[i] + D[i + 1]);
    }
    return C;
  }

  Cubic* CalculateCubicSpline (int n, std::vector<TransferControlPoint> v)
  {
    lqc::Vector4d* gamma = new lqc::Vector4d[n + 1];
    lqc::Vector4d* delta = new lqc::Vector4d[n + 1];
    lqc::Vector4d* D = new lqc::Vector4d[n + 1];
    int i;
   // We need to solve the equation
   // taken from: http://mathworld.wolfram.com/CubicSpline.html
   // [2 1       ] [D[0]]   [3(v[1] - v[0])  ]
   // |1 4 1     | |D[1]|   |3(v[2] - v[0])  |
   // |  1 4 1   | | .  | = |      .         |
   // |    ..... | | .  |   |      .         |
   // |     1 4 1| | .  |   |3(v[n] - v[n-2])|
   // [       1 2] [D[n]]   [3(v[n] - v[n-1])]

    //by converting the matrix to upper triangular.
    //The D[i] are the derivatives at the control points.
    

    //this builds the coefficients of the left matrix
    gamma[0] = lqc::Vector4d::Zero ();
    gamma[0].x = 1.0 / 2.0;
    gamma[0].y = 1.0 / 2.0;
    gamma[0].z = 1.0 / 2.0;
    gamma[0].w = 1.0 / 2.0;
    for (i = 1; i < n; i++)
    {
      gamma[i] = lqc::Vector4d::One () / ((4.0 * lqc::Vector4d::One ()) - gamma[i - 1]);
    }
    gamma[n] = lqc::Vector4d::One () / ((2.0 * lqc::Vector4d::One ()) - gamma[n - 1]);

    delta[0] = 3.0 * (v[1].m_color - v[0].m_color) * gamma[0];
    for (i = 1; i < n; i++)
    {
      delta[i] = (3.0 * (v[i + 1].m_color - v[i - 1].m_color) - delta[i - 1]) * gamma[i];
    }
    delta[n] = (3.0 * (v[n].m_color - v[n - 1].m_color) - delta[n - 1]) * gamma[n];

    D[n] = delta[n];
    for (i = n - 1; i >= 0; i--)
    {
      D[i] = delta[i] - gamma[i] * D[i + 1];
    }

    // now compute the coefficients of the cubics 
    Cubic* C = new Cubic[n];
    for (i = 0; i < n; i++)
    {
      C[i] = Cubic(v[i].m_color, D[i], 3.0 * (v[i + 1].m_color- v[i].m_color) - 2.0 * D[i] - D[i + 1],
        2.0 * (v[i].m_color - v[i + 1].m_color) + D[i] + D[i + 1]);
    }
    return C;
  }
}