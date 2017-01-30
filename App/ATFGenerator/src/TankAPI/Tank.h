#ifndef ATFG_TANK_H
#define ATFG_TANK_H

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

	bool ReadFromVolume(const UINT32& width, const UINT32& height, const UINT32& depth, double* values);

	double GetValue(const UINT32& x, const UINT32& y, const UINT32& z);
	double GetValue(const UINT32& id);

	double CalculateGradient(const UINT32& x, const UINT32& y, const UINT32& z);
	double CalculateLaplacian(const UINT32& x, const UINT32& y, const UINT32& z);
	void CalculateDerivatives(const UINT32& x, const UINT32& y, const UINT32& z, double* g, double* l);

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

private:
	double GetQuadraticGradientNorm(const UINT32& id);

	void FillCellAdjCenter(Cell& cell);
	bool GetSegmentIntersection(const glm::vec3& k, const glm::vec3& l, const glm::vec3& m, const glm::vec3& n, float* s, float* t);
	glm::mat3 GetCellJacobianInverse(const Cell& cell);

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

	double* m_scalar_fx;
	double* m_scalar_fy;
	double* m_scalar_fz;
};

#endif