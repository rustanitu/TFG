/// ConnectivityFilter.h
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#ifndef CONNECTIVITY_FILTER_H
#define CONNECTIVITY_FILTER_H

#include <stack>
#include <volrend/Volume.h>

class ATFGenerator;

class ConnectivityFilter
{
  struct Voxel
  {
    int x, y, z, set;
    bool visited;
    Voxel() : x(0), y(0), z(0), set(0), visited(false) {}
  };

public:
  ConnectivityFilter(vr::Volume* vol, ATFGenerator* atfg);
  ~ConnectivityFilter();

  void SeparateBoundaries();

private:

  int GetNeighborhod(int x, int y, int z, Voxel** neigh);

private:
  int m_width;
  int m_height;
  int m_depth;
  Voxel* m_voxels;
  vr::Volume* m_volume;
  std::stack<Voxel*> m_stack;
  ATFGenerator* m_atfg;
  int m_nsets;
  int* m_set_voxels;
};

#endif