#ifndef ATFG_TANK_H
#define ATFG_TANK_H

#include "Cell.h"

#include <basetsd.h>
#include "ScalarField.h"
#include <vector>

class Tank : public vr::ScalarField
{
public:
	Tank();
	~Tank();

	bool Read(const char* file);

	double GetValue(const UINT32& x, const UINT32& y, const UINT32& z);
	double GetValue(const UINT32& id);

  const Cell& GetCell(const UINT32& x, const UINT32& y, const UINT32& z);
  const Cell& GetCell(const UINT32& id);

  int GetCellsNum()
  {
    return m_ncells;
  }
  
  glm::vec3* GetVertices()
  {
    return m_vertices;
  }

  int GetVerticesNum()
  {
    return m_nvertices;
  }

	double CalculateGradient(const UINT32& x, const UINT32& y, const UINT32& z);
	double CalculateLaplacian(const UINT32& x, const UINT32& y, const UINT32& z);
	void CalculateDerivatives(const UINT32& x, const UINT32& y, const UINT32& z, double* g, double* l);
  void UpdateDerivatives(const UINT32& x, const UINT32& y, const UINT32& z, double* g, double* l);

	void SetCurrentTimeStep(const UINT32& step)
	{
		m_current_timestep = step;
	}

	bool Validate()
	{
		return m_cells != NULL;
	}

	virtual bool IsActive(const UINT32& x, const UINT32& y, const UINT32& z)
	{
		return m_cells[GetId(x, y, z)].IsActive();
	}

  glm::dvec3* GetFaceVertices(const int& x, const int& y, const int& z, const int& face) const;
  glm::dvec3* GetFaceVertices(const Cell& cell, const int& face) const;

  void ResetExtremeDerivatives()
  {
    m_max_gradient = -DBL_MAX;
    m_min_gradient = DBL_MAX;
    m_min_laplacian = DBL_MAX;
    m_max_laplacian = -DBL_MAX;
  }

  virtual bool IsTank()
  {
    return true;
  }

private:
  bool IsParallelPlanes(const glm::dvec3& p0a, const glm::dvec3& p1a, const glm::dvec3& p0b, const glm::dvec3& p1b);
	void FillCellAdjCenter(Cell& cell);
	bool GetSegmentIntersection(const glm::vec3& k, const glm::vec3& l, const glm::vec3& m, const glm::vec3& n, float* s, float* t);
	glm::mat3 GetCellJacobianInverse(const Cell& cell);
  bool IsFaceToFaceCells(const int& x, const int& y, const int& z, const int& i, const int& j, const int& k);

	struct TankComp
	{
		Cell* m_cells;
		bool operator () (const int& cell1, const int& cell2)
		{
			int active1 = 0;
			int active2 = 0;
			for ( int i = 0; i < 6; ++i )
			{
				int idx = m_cells[cell1].GetAdjcentCellIndex(i);
				if ( idx > -1 && m_cells[idx].IsActive() )
					active1++;

				idx = m_cells[cell2].GetAdjcentCellIndex(i);
				if ( idx > -1 && m_cells[m_cells[cell2].GetAdjcentCellIndex(i)].IsActive() )
					active2++;
			}

			return active1 < active2;
		}
	};

private:
	int m_ncells;
	int m_nsteps;
	int m_nvertices;
  glm::vec3* m_vertices;
	Cell* m_cells;
	UINT32 m_current_timestep;
	std::vector<std::string> m_steps;
  glm::mat3* m_hess;
};

#endif