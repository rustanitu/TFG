/// ConnectivityFilter.cpp
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#include "ConnectivityFilter.h"
#include "ATFGenerator.h"
#include "TransferFunction.h"

ConnectivityFilter::ConnectivityFilter(vr::Volume* vol, ATFGenerator* atfg)
: m_width(vol->GetWidth())
, m_height(vol->GetHeight())
, m_depth(vol->GetDepth())
, m_volume(vol)
, m_atfg(atfg)
, m_nsets(0)
{
  int size = m_width * m_height * m_depth;
  m_voxels = new Voxel[size];
  m_set_voxels = new int[size];
  for (int i = 0; i < size; i++)
  {
    m_set_voxels[i] = 0;
  }
}

ConnectivityFilter::~ConnectivityFilter()
{
  delete[] m_voxels;
}

void ConnectivityFilter::SeparateBoundaries(ATFGenerator* atfg)
{
  TransferFunction* tf = (TransferFunction*)m_atfg->GetTransferFunction();

  int set = 0;
  int size = m_width * m_height * m_depth;
  for (int x = 0; x < m_width; x++)
  {
    for (int y = 0; y < m_height; y++)
    {
      for (int z = 0; z < m_depth; z++)
      {
        int id = GETID(x, y, z);
        if (m_voxels[id].visited)
          continue;

        m_voxels[id].x = x;
        m_voxels[id].y = y;
        m_voxels[id].z = z;

        if (!tf->ValidValue(m_volume->SampleVolume(id)))
        {
          m_voxels[id].set = 0;
          m_set_voxels[0]++;
          m_voxels[id].visited = true;
          continue;
        }

        m_voxels[id].set = ++set;
        m_set_voxels[set]++;
        m_voxels[id].visited = true;
        m_stack.push(&m_voxels[id]);

        while (!m_stack.empty())
        {
          Voxel* v = m_stack.top();
          m_stack.pop();

          Voxel** neigh = new Voxel*[18];
          int n = GetNeighborhod(v->x, v->y, v->z, neigh);
          for (int i = 0; i < n; ++i)
          {
            id = GETID(neigh[i]->x, neigh[i]->y, neigh[i]->z);
            if (m_voxels[id].visited)
              continue;

            if (!tf->ValidValue(m_volume->SampleVolume(id)))
            {
              neigh[i]->set = 0;
              m_set_voxels[0]++;
              m_voxels[id].visited = true;
              continue;
            }

            neigh[i]->set = set;
            m_set_voxels[set]++;
            m_voxels[id].visited = true;
            m_stack.push(neigh[i]);
          }

          delete[] neigh;
        }
      }
    }
  }
  m_nsets = set + 1;
}

int ConnectivityFilter::GetNeighborhod(int x, int y, int z, Voxel** neigh)
{
  int nx, ny, nz;
  int count = 0;
  for (int i = 0; i <= 2; i++) {
    for (int j = 0; j <= 2; j++) {
      for (int k = 0; k <= 2; k++) {
        if ((i & 1) | (j & 1) | (k & 1) > 0 && i*j*k != 1) {
          nx = x + i - 1;
          ny = y + j - 1;
          nz = z + k - 1;
          if (nx >= 0 && ny >= 0 && nz >= 0 && nx < m_width && ny < m_height && nz < m_depth) {
            int id = GETID(nx, ny, nz);
            neigh[count] = &m_voxels[id];
            neigh[count]->x = nx;
            neigh[count]->y = ny;
            neigh[count]->z = nz;
            count++;
          }
        }
      }
    }
  }
  return count;
}