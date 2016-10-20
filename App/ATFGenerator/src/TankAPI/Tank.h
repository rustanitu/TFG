#ifndef ATFG_TANK_H
#define ATFG_TANK_H

#include <vector>

class Cell;

class Tank
{
public:
  Tank();
  ~Tank();

  void Clear();
  bool Read(const char* file);

private:
  int m_ncells;
  int m_nsteps;
  int m_nvertices;
  float* m_vertices;
  Cell* m_cells;
  std::vector<std::string> m_steps;
};

#endif