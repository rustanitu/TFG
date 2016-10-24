#ifndef ATFG_TANK_H
#define ATFG_TANK_H

#include <basetsd.h>
#include <volrend/ScalarField.h>
#include <vector>

class Cell;

class Tank : public vr::ScalarField
{
public:
  Tank();
  ~Tank();

  void Clear();
  bool Read(const char* file);

	int GetValue(const UINT32& x, const UINT32& y, const UINT32& z);
	int GetValue(float x, float y, float z);
	int GetValue(const UINT32& id);
  float* GetValues()
  {
    return m_values;
  }

private:
  int m_ncells;
  int m_nsteps;
  int m_nvertices;
  float* m_vertices;
  Cell* m_cells;
  float* m_values;
  std::vector<std::string> m_steps;
};

#endif