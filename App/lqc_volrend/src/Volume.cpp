#include <volrend/Volume.h>

#include <lqc/File/RAWLoader.h>
#include <math/MUtils.h>
#include <math/MSVD.h>
#include <math/MGeometry.h>
#include <volrend/TransferFunction.h>

#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>

#define sqrte 1.6487212707

namespace vr
{
  int SOBEL_X_MASK[27] =
  {
    -1, -2, -1, -2, -4, -2, -1, -2, -1,
     0,  0,  0,  0,  0,  0,  0,  0,  0,
     1,  2,  1,  2,  4,  2,  1,  2,  1
  };

  int SOBEL_Y_MASK[27] =
  {
    -1, -2, -1,  0,  0,  0,  1,  2,  1,
    -2, -4, -2,  0,  0,  0,  2,  4,  2,
    -1, -2, -1,  0,  0,  0,  1,  2,  1
  };

  int SOBEL_Z_MASK[27] =
  {
    -1,  0,  1, -2,  0,  2, -1,  0,  1,
    -2,  0,  2, -4,  0,  4, -2,  0,  2,
    -1,  0,  1, -2,  0,  2, -1,  0,  1
  };

  int LAPLACE_MASK[27] =
  {
    0,  0,  0,  0,  1,  0,  0,  0,  0,
    0,  1,  0,  0, -6,  0,  0,  1,  0,
    0,  0,  0,  0,  1,  0,  0,  0,  0
  };

  Volume::Volume ()
  : m_width(0), m_height(0), m_depth(0), m_max_gradient(0), m_min_laplacian(0), m_max_laplacian(0),
    m_scalar_values(NULL), m_scalar_gradient(NULL), m_scalar_laplacian(NULL), m_scalar_histogram(NULL)
  {
  }

  Volume::Volume (unsigned int width, unsigned int height, unsigned int depth)
  : m_width(width), m_height(height), m_depth(depth), m_max_gradient(0), m_min_laplacian(0), m_max_laplacian(0), m_scalar_histogram(NULL)
  {
    m_scalar_values = NULL;
    m_scalar_values = new float[m_width*m_height*m_depth];

    m_scalar_gradient = NULL;
    m_scalar_gradient = new float[m_width*m_height*m_depth];
    
    m_scalar_laplacian = NULL;
    m_scalar_laplacian = new float[m_width*m_height*m_depth];

    for (int i = 0; i < (int)(width*height*depth); i++)
    {
      m_scalar_values[i] = 0.0f;
      m_scalar_gradient[i] = 0;
      m_scalar_laplacian[i] = 0;
    }
  }

  Volume::Volume (unsigned int width, unsigned int height, unsigned int depth, float* scalars)
  : m_width(width), m_height(height), m_depth(depth), m_max_gradient(0), m_min_laplacian(0), m_max_laplacian(0), m_scalar_histogram(NULL)
  {
    m_scalar_values = NULL;
    m_scalar_values = new float[m_width*m_height*m_depth];

    m_scalar_gradient = NULL;
    m_scalar_gradient = new float[m_width*m_height*m_depth];

    m_scalar_laplacian = NULL;
    m_scalar_laplacian = new float[m_width*m_height*m_depth];

    if (scalars != NULL && m_scalar_gradient != NULL)
    {
      for (int i = 0; i < (int)(width*height*depth); i++)
      {
        m_scalar_values[i] = (float)scalars[i];
        m_scalar_gradient[i] = 0;
        m_scalar_laplacian[i] = 0;
      }
    }

    FillGradientField();
    FillLaplacianField();
    if (GenerateHistogram())
      GenerateTransferFunction();
  }

  Volume::~Volume ()
  {
    delete[] m_scalar_values;
  }

  int Volume::GetWidth ()
  {
    return m_width;
  }

  int Volume::GetHeight ()
  {
    return m_height;
  }

  int Volume::GetDepth ()
  {
    return m_depth;
  }
  
  lqc::Vector3f Volume::GetAnchorMin ()
  {
    return m_pmin;
  }

  lqc::Vector3f Volume::GetAnchorMax ()
  {
    return m_pmax;
  }

  void Volume::SetAnchors (lqc::Vector3f pmin, lqc::Vector3f pmax)
  {
    m_pmin = pmin;
    m_pmax = pmax;
  }

  int Volume::SampleVolume(int x, int y, int z)
  {
    x = lqc::Clamp (x, 0, m_width - 1);
    y = lqc::Clamp (y, 0, m_height - 1);
    z = lqc::Clamp (z, 0, m_depth - 1);

    return (int)m_scalar_values[x + (y * m_width) + (z * m_width * m_height)];
  }

  int Volume::SampleVolume (float x, float y, float z)
  {
    if ((x >= m_pmin.x && x <= m_pmax.x) && (y >= m_pmin.y && y <= m_pmax.y) && (z >= m_pmin.z && z <= m_pmax.z))
      return -1;

    int px = (int)x;
    int py = (int)y;
    int pz = (int)z;

    return (int)m_scalar_values[px + (py * m_width) + (pz * m_width * m_height)];
  }

  float Volume::InterpolatedValue (float px, float py, float pz)
  {
    float x = ((px - m_pmin.x) / (m_pmax.x - m_pmin.x)) * (float)(m_width - 1);
    float y = ((py - m_pmin.y) / (m_pmax.y - m_pmin.y)) * (float)(m_height - 1);
    float z = ((pz - m_pmin.z) / (m_pmax.z - m_pmin.z)) * (float)(m_depth - 1);

    int x0 = (int)x; int x1 = x0 + 1;
    int y0 = (int)y; int y1 = y0 + 1;
    int z0 = (int)z; int z1 = z0 + 1;

    if (x0 == (float)(m_width-1))
    {
      x1 = (int)x0;
      x0 = x1 - 1;
    }

    if (y0 == (float)(m_height - 1))
    {
      y1 = (int)y0;
      y0 = y1 - 1;
    }

    if (z0 == (float)(m_depth - 1))
    {
      z1 = (int)z0;
      z0 = z1 - 1;
    }
    
    float xd = (x - (float)x0) / (float)(x1 - x0);
    float yd = (y - (float)y0) / (float)(y1 - y0);
    float zd = (z - (float)z0) / (float)(z1 - z0);
        
    // X interpolation
    float c00 = SampleVolume (x0, y0, z0)*(1.0f - xd)
      + SampleVolume (x1, y0, z0)*xd;

    float c10 = SampleVolume (x0, y1, z0)*(1.0f - xd)
      + SampleVolume (x1, y1, z0)*xd;

    float c01 = SampleVolume (x0, y0, z1)*(1.0f - xd)
      + SampleVolume (x1, y0, z1)*xd;

    float c11 = SampleVolume (x0, y1, z1)*(1.0f - xd)
      + SampleVolume (x1, y1, z1)*xd;

    // Y interpolation
    float c0 = c00*(1.0f - yd) + c10*yd;
    float c1 = c01*(1.0f - yd) + c11*yd;

    // Z interpolation
    float c = c0*(1.0f - zd) + c1*zd;

    return c;
  }

  float Volume::InterpolatedValue (lqc::Vector3f pos)
  {
    return InterpolatedValue (pos.x, pos.y, pos.z);
  }

  float Volume::TrilinearScalarFunction (lqc::Vector3f pos, lqc::Vector3f rayeye, lqc::Vector3f raydirection)
  {
    float fx = pos.x, fy = pos.y, fz = pos.z;

    fx = (fx - m_pmin.x) / abs (m_pmax.x - m_pmin.x) * m_width;
    fy = (fy - m_pmin.y) / abs (m_pmax.y - m_pmin.y) * m_height;
    fz = (fz - m_pmin.z) / abs (m_pmax.z - m_pmin.z) * m_depth;

    int x = (int)fx;
    int y = (int)fy;
    int z = (int)fz;

    if (x == m_width) --x;
    if (y == m_height) --y;
    if (z == m_depth) --z;

    //////////////////////////////////////////////////////////////////////////////
    //f(x, y, z) = c0 + c1*x + c2*y + c3*z + c4*x*y + c5*y*z + c6*x*z + c7*x*y*z//
    //////////////////////////////////////////////////////////////////////////////

    //cell coeficients
    float c[8];
    //scalar values of the cells
    float s[8];
    int m = 8, n = 8;
    int px[8] = { x, x + 1, x, x + 1, x, x + 1, x, x + 1 };
    int py[8] = { y, y, y, y, y + 1, y + 1, y + 1, y + 1 };
    int pz[8] = { z, z, z + 1, z + 1, z, z, z + 1, z + 1 };
    for (int i = 0; i < n; i++)
      s[i] = (float)SampleVolume (px[i], py[i], pz[i]);

    //mallocs
    float **a = (float**)malloc (sizeof(float*)* m);
    for (int i = 0; i < m; i++)
    {
      a[i] = (float*)malloc (sizeof(float)* n);
      a[i][0] = 1;
      a[i][1] = (float)px[i];
      a[i][2] = (float)py[i];
      a[i][3] = (float)pz[i];
      a[i][4] = (float)px[i] * py[i];
      a[i][5] = (float)py[i] * pz[i];
      a[i][6] = (float)px[i] * pz[i];
      a[i][7] = (float)px[i] * py[i] * pz[i];
    }
    float *svdw = (float*)malloc (sizeof(float)* n);
    float **v = (float**)malloc (sizeof(float*)* n);
    for (int i = 0; i < n; i++)
      v[i] = (float*)malloc (sizeof(float)* n);

    //Make SVD on Matrix a
    lqc::dcksvd (a, m, n, svdw, v);

    //////////////////////////////////////////////////////////////
    //f(t) = w3*(t*t*t) + w2*(t*t) + w1*t + w0, t[tback, tfront]//
    //////////////////////////////////////////////////////////////

    //TODO
    float t = 0.0f;

    lqc::Vector3f e = rayeye;
    lqc::Vector3f d = raydirection;

    float w[4];
    w[0] = c[0] + c[1] * e.x + c[2] * e.y + c[4] * e.x*e.y + c[3] * e.z
      + c[6] * e.x*e.z + c[5] * e.y*e.z + c[7] * e.x*e.y*e.z
      + c[7] * d.x*d.y*e.z;
    w[1] = c[1] * d.x + c[2] * d.y + c[3] * d.z + c[4] * d.y*e.x + c[6] * d.z*e.x
      + c[4] * d.x*e.y + c[5] * d.z*e.y + c[7] * d.z*e.x*e.y + c[6] * d.x*e.z + c[5] * d.y*e.z
      + c[7] * d.y*e.x*e.z + c[7] * d.x*e.y*e.z;
    w[2] = c[4] * d.x*d.y + c[6] * d.x*d.z + c[5] * d.y*d.z + c[7] * d.y*d.z*e.x + c[7] * d.x*d.z*e.y;
    w[3] = c[7] * d.x*d.y*d.z;

    float f_t = w[3] * (t*t*t) + w[2] * (t*t) + w[1] * t + w[0];

    //frees
    for (int i = 0; i < m; i++)
      free (a[i]);
    free (a);

    for (int i = 0; i < n; i++)
      free (v[i]);
    free (v);

    free (svdw);

    // return f_t;
    return 0.0f;
  }

  bool Volume::IsOutOfBoundary (int x, int y, int z)
  {
    return !((x >= 0 && x < m_width)
          && (y >= 0 && y < m_height)
          && (z >= 0 && z < m_depth));
  }

  
  /////////////////////
  // Private Methods //
  /////////////////////

  float Volume::CalculateGradient (int x, int y, int z)
  {
    int s = 0;
    float gx = 0;
    float gy = 0;
    float gz = 0;
    for (int i = x - 1; i <= x + 1; ++i)
    {
      for (int j = y - 1; j <= y + 1; ++j)
      {
        for (int k = z - 1; k <= z + 1; ++k)
        {
          float v = GetValue(i, j, k);
          gx += SOBEL_X_MASK[s] * v;
          gy += SOBEL_Y_MASK[s] * v;
          gz += SOBEL_Z_MASK[s] * v;
          ++s;
        }
      }
    }

    gx /= 16.0f;
    gy /= 16.0f;
    gz /= 16.0f;
    
    float g = sqrt(gx*gx + gy*gy + gz*gz);
    if (g > m_max_gradient)
      m_max_gradient = g + 0.5f;

    return g;
  }

  float Volume::CalculateLaplacian (int x, int y, int z)
  {
    int s = 0;
    float lx = 0;
    float ly = 0;
    float lz = 0;
    for (int i = x - 1; i <= x + 1; ++i)
    {
      for (int j = y - 1; j <= y + 1; ++j)
      {
        for (int k = z - 1; k <= z + 1; ++k)
        {
          //float v = GetValue(i, j, k);
          //lx += SOBEL_X_MASK[s] * v;
          //ly += SOBEL_Y_MASK[s] * v;
          //lz += SOBEL_Z_MASK[s] * v;
          //++s;
          lx += LAPLACE_MASK[s] * GetValue(i, j, k);
          ++s;
        }
      }
    }

    //lx /= 16.0f;
    //ly /= 16.0f;
    //lz /= 16.0f;

    float l = lx;// sqrt(lx*lx + ly*ly + lz*lz);
    if (l > m_max_laplacian)
      m_max_laplacian = l + 0.5f;
    if (l < m_min_laplacian)
      m_min_laplacian = l - 0.5f;

    return l;
  }

  float Volume::GetValue(int x, int y, int z)
  {
    x = lqc::Clamp(x, 0, m_width - 1);
    y = lqc::Clamp(y, 0, m_height - 1);
    z = lqc::Clamp(z, 0, m_depth - 1);

    return m_scalar_values[x + (y * m_width) + (z * m_width * m_height)];
  }

  float Volume::GetGradient(int x, int y, int z)
  {
    x = lqc::Clamp(x, 0, m_width - 1);
    y = lqc::Clamp(y, 0, m_height - 1);
    z = lqc::Clamp(z, 0, m_depth - 1);

    return m_scalar_gradient[x + (y * m_width) + (z * m_width * m_height)];
  }

  float Volume::GetLaplacian(int x, int y, int z)
  {
    x = lqc::Clamp(x, 0, m_width - 1);
    y = lqc::Clamp(y, 0, m_height - 1);
    z = lqc::Clamp(z, 0, m_depth - 1);

    return m_scalar_laplacian[x + (y * m_width) + (z * m_width * m_height)];
  }

  void Volume::FillGradientField()
  {
    for (int x = 1; x < m_width - 1; x++)
    {
      for (int y = 1; y < m_height - 1; y++)
      {
        for (int z = 1; z < m_depth - 1; z++)
        {
          m_scalar_gradient[x + (y * m_width) + (z * m_width * m_height)] = CalculateGradient(x, y, z);
          m_scalar_laplacian[x + (y * m_width) + (z * m_width * m_height)] = CalculateLaplacian(x, y, z);
        }
      }
    }
  }

  void Volume::FillLaplacianField()
  {
    return;
    for (int x = 1; x < m_width - 1; x++)
    {
      for (int y = 1; y < m_height - 1; y++)
      {
        for (int z = 1; z < m_depth - 1; z++)
        {
          m_scalar_laplacian[x + (y * m_width) + (z * m_width * m_height)] = CalculateLaplacian(x, y, z);
        }
      }
    }
  }

  float* Volume::GetBoundaryDistancies()
  {
    float sigma = m_max_gradient / ((float)m_max_laplacian * sqrte);

    float x[256];

    for (size_t v = 0; v < 256; ++v)
    {
      float g = m_average_gradient[v];
      float l = m_average_laplacian[v + 256 * (unsigned int)g];
      if (g <= 0.0f)
      {
        x[v] = 0.0f;
        continue;
      }

      x[v] = -(sigma * sigma * l) / g;
    }

    return x;
  }

  bool Volume::GenerateHistogram()
  {
    if (m_scalar_histogram)
      delete m_scalar_histogram;

    unsigned long size = 256 * (m_max_gradient + 1) * (m_max_laplacian - m_min_laplacian + 1);
    m_scalar_histogram = new unsigned char[size];
    if (!m_scalar_histogram)
    {
      printf("Insuficient memory to generate histogram!\n");
      return false;
    }

    for (unsigned int i = 0; i < size; ++i)
    {
      m_scalar_histogram[i] = 0;
    }

    size = 256 * (m_max_gradient + 1);
    m_gradient_qtd = new unsigned int[size];
    m_average_laplacian = new float[size];
    if (!m_gradient_qtd || !m_average_laplacian)
    {
      printf("Insuficient memory to generate histogram!\n");
      return false;
    }

    for (unsigned int i = 0; i < size; ++i)
    {
      m_gradient_qtd[i] = 0;
      m_average_laplacian[i] = -FLT_MAX;
    }

    for (size_t i = 0; i < 256; i++)
    {
      m_average_gradient[i] = 0;
      m_value_qtd[i] = 0;
    }

    //alocate avereage h(g,v)

    for (int x = 0; x < m_width; x++)
    {
      for (int y = 0; y < m_height; y++)
      {
        for (int z = 0; z < m_depth; z++)
        {
          unsigned int vol_id = x + (y * m_width) + (z * m_width * m_height);
          float v = m_scalar_values[vol_id];
          float g = m_scalar_gradient[vol_id];
          float l = m_scalar_laplacian[vol_id];

          if (v < 0.0f || v > 255.0f)
            continue;

          unsigned int vid = v;
          m_average_gradient[vid] += g;
          ++m_value_qtd[vid];

          unsigned long lid = v + g * 256;
          if (m_average_laplacian[lid] == -FLT_MAX)
            m_average_laplacian[lid] = l;
          else
            m_average_laplacian[lid] += l;
          ++m_gradient_qtd[lid];

          unsigned long hid = lid + ((l - m_min_laplacian) * 256 * (m_max_gradient + 1));
          if (m_scalar_histogram[hid] < 255)
            ++m_scalar_histogram[hid];
        }
      }
    }

    for (size_t i = 0; i < 256; i++)
    {
      if (m_value_qtd[i] > 0)
        m_average_gradient[i] /= m_value_qtd[i];
    }

    for (unsigned long i = 0; i < size; i++)
    {
      if (m_gradient_qtd[i] > 0)
        m_average_laplacian[i] /= m_gradient_qtd[i];
    }

    return true;
  }

  void Volume::GenerateTransferFunction()
  {
    std::ofstream state_file;
    state_file.open("../../Modelos//TransferFunctions//AutomaticTransferFunction.tf1d");
    
    state_file << "linear" << "\n";
    state_file << "0" << "\n";
    state_file << "9" << "\n";
    state_file << "1   1   1   0" << "\n";
    state_file << "1   0   0   32" << "\n";
    state_file << "0   1   0   64" << "\n";
    state_file << "0   0   1   96" << "\n";
    state_file << "0.5 0.5 0   128" << "\n";
    state_file << "0.5 0   0.5 160" << "\n";
    state_file << "0   0.5 0.5 192" << "\n";
    state_file << "0.3 0.3 0.3 224" << "\n";
    state_file << "0   0   0   256" << "\n";
    state_file << 256 << "\n";

    float* x = GetBoundaryDistancies();
    if (!x)
    {
      state_file.close();
      return;
    }

    for (size_t v = 0; v < 256; ++v)
    {
      float base = 0.8f;
      if (x[v] >= -base && x[v] <= base)
      {
        float a = 0.0f;
        float amax = 0.5f;
        if (x[v] >= 0.0)
          a = -(amax * x[v]) / base;
        else
          a = (amax * x[v]) / base;

        a += amax;
        state_file << a << "\t" << v << "\n";
      }
      else
        state_file << 0 << "\t" << v << "\n";
    }
    state_file.close();
  }
}