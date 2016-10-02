#include <volrend/Volume.h>

#include <lqc/File/RAWLoader.h>
#include <math/MUtils.h>
#include <math/MSVD.h>
#include <math/MGeometry.h>

#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>

namespace vr
{

  Volume::Volume()
    : m_width(0), m_height(0), m_depth(0), m_nsets(0)
  {
    m_scalar_values = NULL;
    m_visited = NULL;
    m_set_values = NULL;
    m_set_qtd = NULL;
  }

  Volume::Volume(unsigned int width, unsigned int height, unsigned int depth)
    : m_width(width), m_height(height), m_depth(depth), m_nsets(0)
  {
    int size = m_width*m_height*m_depth;

    m_scalar_values = new float[size];
    m_visited = new bool[size];
    m_set_qtd = new int[size];
    m_set_values = new int[size];

    for (int i = 0; i < size; i++)
    {
      m_scalar_values[i] = 0.0f;
      m_visited[i] = false;
      m_set_qtd[i] = 0;
      m_set_values[i] = 0;
    }
  }

  Volume::Volume(unsigned int width, unsigned int height, unsigned int depth, float* scalars)
    : m_width(width), m_height(height), m_depth(depth), m_nsets(0)
  {
    int size = m_width*m_height*m_depth;

    m_scalar_values = new float[size];
    m_visited = new bool[size];
    m_set_qtd = new int[size];
    m_set_values = new int[size];

    for (int i = 0; i < size; i++)
    {
      m_scalar_values[i] = (float)scalars[i];
      m_visited[i] = false;
      m_set_qtd[i] = 0;
      m_set_values[i] = 0;
    }
  }

  Volume::~Volume()
  {
    delete[] m_scalar_values;
    delete[] m_visited;
    delete[] m_set_qtd;
    delete[] m_set_values;
  }

  int Volume::GetWidth()
  {
    return m_width;
  }

  int Volume::GetHeight()
  {
    return m_height;
  }

  int Volume::GetDepth()
  {
    return m_depth;
  }

  lqc::Vector3f Volume::GetAnchorMin()
  {
    return m_pmin;
  }

  lqc::Vector3f Volume::GetAnchorMax()
  {
    return m_pmax;
  }

  void Volume::SetAnchors(lqc::Vector3f pmin, lqc::Vector3f pmax)
  {
    m_pmin = pmin;
    m_pmax = pmax;
  }

  void Volume::SeparateBoundaries(vr::TransferFunction1D* tf)
  {
    return;
    int set = 0;
    int size = m_width * m_height * m_depth;
    for (UINT x = 0; x < m_width; x++)
    {
      for (UINT y = 0; y < m_height; y++)
      {
        for (UINT z = 0; z < m_depth; z++)
        {
          int id = GetId(x, y, z);
          if (m_visited[id])
            continue;

          Voxel vi;
          vi.x = x;
          vi.y = y;
          vi.z = z;

          if (!tf->ValidValue(SampleVolume(id)))
          {
            m_set_values[id] = 0;
            m_set_qtd[0]++;
            m_visited[id] = true;
            continue;
          }

          m_set_values[id] = ++set;
          m_set_qtd[set]++;
          m_visited[id] = true;
          m_stack.push(vi);

          while (!m_stack.empty())
          {
            Voxel v = m_stack.top();
            m_stack.pop();

            Voxel* neigh = new Voxel[18];
            int n = GetNeighborhod(v.x, v.y, v.z, neigh);
            for (int i = 0; i < n; ++i)
            {
              id = GetId(neigh[i].x, neigh[i].y, neigh[i].z);
              if (m_visited[id])
                continue;

              if (!tf->ValidValue(SampleVolume(id)))
              {
                m_set_values[id] = 0;
                m_set_qtd[0]++;
                m_visited[id] = true;
                continue;
              }

              m_set_values[id] = set;
              m_set_qtd[set]++;
              m_visited[id] = true;
              m_stack.push(neigh[i]);
            }

            delete[] neigh;
          }
        }
      }
    }
    m_nsets = set + 1;
  }

  int Volume::GetNeighborhod(int x, int y, int z, Voxel* neigh)
  {
    int nx, ny, nz;
    int count = 0;
    for (int i = 0; i <= 2; i++)
    {
      for (int j = 0; j <= 2; j++)
      {
        for (int k = 0; k <= 2; k++)
        {
          if ((i & 1) | (j & 1) | (k & 1) > 0 && i*j*k != 1)
          {
            nx = x + i - 1;
            ny = y + j - 1;
            nz = z + k - 1;
            if (nx >= 0 && ny >= 0 && nz >= 0 && nx < m_width && ny < m_height && nz < m_depth)
            {
              int id = GetId(nx, ny, nz);
              neigh[count].x = nx;
              neigh[count].y = ny;
              neigh[count].z = nz;
              count++;
            }
          }
        }
      }
    }
    return count;
  }

  int Volume::SampleVolume(const unsigned int& x, const unsigned int& y, const unsigned int& z)
  {
    unsigned int xt = lqc::Clamp(x, 0, m_width - 1);
    unsigned int yt = lqc::Clamp(y, 0, m_height - 1);
    unsigned int zt = lqc::Clamp(z, 0, m_depth - 1);

    return (int)m_scalar_values[GetId(xt, yt, zt)];
  }

  int Volume::SampleVolume(float x, float y, float z)
  {
    if ((x >= m_pmin.x && x <= m_pmax.x) && (y >= m_pmin.y && y <= m_pmax.y) && (z >= m_pmin.z && z <= m_pmax.z))
      return -1;

    int px = (int)x;
    int py = (int)y;
    int pz = (int)z;

    return (int)m_scalar_values[GetId(px, py, pz)];
  }

  float Volume::InterpolatedValue(float px, float py, float pz)
  {
    float x = ((px - m_pmin.x) / (m_pmax.x - m_pmin.x)) * (float)(m_width - 1);
    float y = ((py - m_pmin.y) / (m_pmax.y - m_pmin.y)) * (float)(m_height - 1);
    float z = ((pz - m_pmin.z) / (m_pmax.z - m_pmin.z)) * (float)(m_depth - 1);

    unsigned int x0 = (int)x; unsigned int x1 = x0 + 1;
    unsigned int y0 = (int)y; unsigned int y1 = y0 + 1;
    unsigned int z0 = (int)z; unsigned int z1 = z0 + 1;

    if (x0 == (float)(m_width - 1))
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
    float c00 = SampleVolume(x0, y0, z0)*(1.0f - xd)
      + SampleVolume(x1, y0, z0)*xd;

    float c10 = SampleVolume(x0, y1, z0)*(1.0f - xd)
      + SampleVolume(x1, y1, z0)*xd;

    float c01 = SampleVolume(x0, y0, z1)*(1.0f - xd)
      + SampleVolume(x1, y0, z1)*xd;

    float c11 = SampleVolume(x0, y1, z1)*(1.0f - xd)
      + SampleVolume(x1, y1, z1)*xd;

    // Y interpolation
    float c0 = c00*(1.0f - yd) + c10*yd;
    float c1 = c01*(1.0f - yd) + c11*yd;

    // Z interpolation
    float c = c0*(1.0f - zd) + c1*zd;

    return c;
  }

  float Volume::InterpolatedValue(lqc::Vector3f pos)
  {
    return InterpolatedValue(pos.x, pos.y, pos.z);
  }

  float Volume::TrilinearScalarFunction(lqc::Vector3f pos, lqc::Vector3f rayeye, lqc::Vector3f raydirection)
  {
    float fx = pos.x, fy = pos.y, fz = pos.z;

    fx = (fx - m_pmin.x) / abs(m_pmax.x - m_pmin.x) * m_width;
    fy = (fy - m_pmin.y) / abs(m_pmax.y - m_pmin.y) * m_height;
    fz = (fz - m_pmin.z) / abs(m_pmax.z - m_pmin.z) * m_depth;

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
    unsigned int px[8] = { x, x + 1, x, x + 1, x, x + 1, x, x + 1 };
    unsigned int py[8] = { y, y, y, y, y + 1, y + 1, y + 1, y + 1 };
    unsigned int pz[8] = { z, z, z + 1, z + 1, z, z, z + 1, z + 1 };
    for (int i = 0; i < n; i++)
      s[i] = (float)SampleVolume(px[i], py[i], pz[i]);

    //mallocs
    float **a = (float**)malloc(sizeof(float*)* m);
    for (int i = 0; i < m; i++)
    {
      a[i] = (float*)malloc(sizeof(float)* n);
      a[i][0] = 1;
      a[i][1] = (float)px[i];
      a[i][2] = (float)py[i];
      a[i][3] = (float)pz[i];
      a[i][4] = (float)px[i] * py[i];
      a[i][5] = (float)py[i] * pz[i];
      a[i][6] = (float)px[i] * pz[i];
      a[i][7] = (float)px[i] * py[i] * pz[i];
    }
    float *svdw = (float*)malloc(sizeof(float)* n);
    float **v = (float**)malloc(sizeof(float*)* n);
    for (int i = 0; i < n; i++)
      v[i] = (float*)malloc(sizeof(float)* n);

    //Make SVD on Matrix a
    lqc::dcksvd(a, m, n, svdw, v);

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
      free(a[i]);
    free(a);

    for (int i = 0; i < n; i++)
      free(v[i]);
    free(v);

    free(svdw);

    // return f_t;
    return 0.0f;
  }

  bool Volume::IsOutOfBoundary(int x, int y, int z)
  {
    return !((x >= 0 && x < m_width)
             && (y >= 0 && y < m_height)
             && (z >= 0 && z < m_depth));
  }

  /////////////////////
  // Private Methods //
  /////////////////////

}