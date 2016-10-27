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

  float GetValue(const UINT32& x, const UINT32& y, const UINT32& z);
  float GetValue(const UINT32& id);

	float CalculateGradient(const UINT32& x, const UINT32& y, const UINT32& z);
	float CalculateLaplacian(const UINT32& x, const UINT32& y, const UINT32& z);

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