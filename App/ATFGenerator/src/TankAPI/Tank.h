#ifndef ATFG_TANK_H
#define ATFG_TANK_H

#include "../AutomaticTransferFunction/DerivativeMask.h"
#include "Cell.h"

#include <basetsd.h>
#include <volrend/ScalarField.h>
#include <vector>

class Tank : public vr::ScalarField
{
public:
  Tank();
  ~Tank();

  bool Read(const char* file);

  float GetValue(const UINT32& x, const UINT32& y, const UINT32& z);
  float GetValue(const UINT32& id);

  float CalculateGradient(const UINT32& x, const UINT32& y, const UINT32& z);
  float CalculateLaplacian(const UINT32& x, const UINT32& y, const UINT32& z);

  void SetCurrentTimeStep(const UINT32& step)
  {
    m_current_timestep = step;
  }

  bool Validate()
  {
    return m_cells != NULL;
  }

private:
  float GetQuadraticGradientNorm(const UINT32& id);

  struct TankComp
  {
    Cell* m_cells;
    bool operator () (const int& cell1, const int& cell2)
    {
      int active1 = 0;
      int active2 = 0;
      for (int i = 0; i < 6; ++i) {
        int idx = m_cells[cell1].GetAdjcentCellIndex(i);
        if (idx > -1 && m_cells[idx].IsActive())
          active1++;

        idx = m_cells[cell2].GetAdjcentCellIndex(i);
        if (idx > -1 && m_cells[m_cells[cell2].GetAdjcentCellIndex(i)].IsActive())
          active2++;
      }

      return active1 < active2;
    }
  };

private:
  int m_ncells;
  int m_nsteps;
  int m_nvertices;
  float* m_vertices;
  Cell* m_cells;
  UINT32 m_current_timestep;
  std::vector<std::string> m_steps;
  DerivativeMask m_derivativeMask;

  float* m_scalar_fx;
  float* m_scalar_fy;
  float* m_scalar_fz;
};

#endif