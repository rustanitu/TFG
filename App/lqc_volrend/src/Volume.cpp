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

namespace vr
{
  int SOBEL_MASK[27] =
  {
    -1, -2, -1, -2, -4, -2, -1, -2, -1,
     0,  0,  0,  0,  0,  0,  0,  0,  0,
     1,  2,  1,  2,  4,  2,  1,  2,  1
  };

  Volume::Volume ()
    : m_width (0), m_height (0), m_depth (0)
  {
    m_scalar_values = NULL;
    m_scalar_gradient = NULL;
  }

  Volume::Volume (unsigned int width, unsigned int height, unsigned int depth)
    : m_width (width), m_height (height), m_depth (depth)
  {
    m_scalar_values = NULL;
    m_scalar_values = new float[m_width*m_height*m_depth];

    m_scalar_gradient = NULL;
    m_scalar_gradient = new float[m_width*m_height*m_depth];;

    for (int i = 0; i < (int)(width*height*depth); i++)
    {
      m_scalar_values[i] = 0.0f;
      m_scalar_gradient[i] = 0.0f;
    }
  }

  Volume::Volume (unsigned int width, unsigned int height, unsigned int depth, float* scalars)
  : m_width (width), m_height (height), m_depth (depth)
  {
    m_scalar_values = NULL;
    m_scalar_values = new float[m_width*m_height*m_depth];

    m_scalar_gradient = NULL;
    m_scalar_gradient = new float[m_width*m_height*m_depth];;

    if (scalars != NULL && m_scalar_gradient != NULL)
    {
      for (int i = 0; i < (int)(width*height*depth); i++)
      {
        m_scalar_values[i] = (float)scalars[i];
        m_scalar_gradient[i] = 0.0f;
      }
    }
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

  int Volume::SampleGradient (int x, int y, int z)
  {
    x = lqc::Clamp(x, 0, m_width - 1);
    y = lqc::Clamp(y, 0, m_height - 1);
    z = lqc::Clamp(z, 0, m_depth - 1);

    return (int)m_scalar_gradient[x + (y * m_width) + (z * m_width * m_height)];
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

  void Volume::FillGradientField ()
  {
    for (int x = 1; x < m_width - 1; x++)
    {
      for (int y = 1; y < m_height - 1; y++)
      {
        for (int z = 1; z < m_depth - 1; z++)
        {
          m_scalar_gradient[x + (y * m_width) + (z * m_width * m_height)] = GradientSample(x, y, z);
        }
      }
    }
  }

  /////////////////////
  // Private Methods //
  /////////////////////

  int Volume::GradientSample(int x, int y, int z)
  {
    int s;
    
    // X direction
    s = 0;
    int gx = 0;
    for (int i = x - 1; i <= x + 1; ++i)
    {
      for (int j = y - 1; j <= y + 1; ++j)
      {
        for (int k = z - 1; k <= z + 1; ++k)
        {
          gx += SOBEL_MASK[s++] * SampleVolume(i, j, k);
        }
      }
    }

    // Y direction
    s = 0;
    int gy = 0;
    for (int j = y - 1; j <= y + 1; ++j)
    {
      for (int i = x - 1; i <= x + 1; ++i)
      {
        for (int k = z - 1; k <= z + 1; ++k)
        {
          gy += SOBEL_MASK[s++] * SampleVolume(i, j, k);
        }
      }
    }

    // Z direction
    s = 0;
    int gz = 0;
    for (int k = z - 1; k <= z + 1; ++k)
    {
      for (int i = x - 1; i <= x + 1; ++i)
      {
        for (int j = y - 1; j <= y + 1; ++j)
        {
          gz += SOBEL_MASK[s++] * SampleVolume(i, j, k);
        }
      }
    }

    return gx + gy + gz;
  }
}