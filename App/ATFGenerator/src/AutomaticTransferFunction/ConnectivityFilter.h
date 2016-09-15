/// ConnectivityFilter.h
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#ifndef CONNECTIVITY_FILTER_H
#define CONNECTIVITY_FILTER_H

#include <stack>
#include <volrend/Volume.h>

#define GETID(x, y, z) x + (y * m_width) + (z * m_width * m_height)

class ConnectivityFilter
{
  struct Voxel
  {
    int x, y, z, id;
  };

public:
  ConnectivityFilter(vr::Volume* vol);
  ~ConnectivityFilter();

  void SeparateBoundaries();

private:

  int GetNeighborhod(int x, int y, int z, Voxel* neigh);

private:
  int m_width;
  int m_height;
  int m_depth;
  bool* m_volume;
  std::stack<Voxel> m_stack;
};

#endif