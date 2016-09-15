/// ConnectivityFilter.cpp
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#include "ConnectivityFilter.h"

ConnectivityFilter::ConnectivityFilter(vr::Volume* vol)
: m_width(vol->GetWidth())
, m_height(vol->GetHeight())
, m_depth(vol->GetDepth())
{
  m_volume = new bool[vol->GetWidth() * vol->GetHeight() * vol->GetDepth()];

  for (int i = 0; i < m_width; i++) {
    for (int j = 0; j < m_height; j++) {
      for (int k = 0; k < m_depth; k++) {
        m_volume[GETID(i, j, k)] = false;
      }
    }
  }
}

ConnectivityFilter::~ConnectivityFilter()
{
}

void ConnectivityFilter::SeparateBoundaries()
{
  Voxel init;
  init.x = m_width/2;
  init.y = m_height/2;
  init.z = m_depth/2;
  init.id = GETID(init.x, init.y, init.z);

  m_volume[init.id] = true;
  m_stack.push(init);

  while (!m_stack.empty()) {
    Voxel v = m_stack.top();
    m_stack.pop();

    Voxel neigh[18];
    int n = GetNeighborhod(v.x, v.y, v.z, neigh);
    for (int i = 0; i < n; ++i) {
      if (m_volume[neigh[i].id])
        continue;

      // Verify if not a valid voxel
      // then continue;

      m_volume[neigh[i].id] = true;
      m_stack.push(neigh[i]);
    }
  }
}

int ConnectivityFilter::GetNeighborhod(int x, int y, int z, Voxel* neigh)
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
            neigh[count++].x = nx;
            neigh[count++].y = ny;
            neigh[count++].z = nz;
            neigh[count++].id = GETID(nx, ny, nz);
          }
        }
      }
    }
  }

  return count;
}