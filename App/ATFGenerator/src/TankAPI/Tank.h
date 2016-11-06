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
	int m_ncells;
	int m_nsteps;
	int m_nvertices;
	float* m_vertices;
	Cell* m_cells;
	UINT32 m_current_timestep;
	std::vector<std::string> m_steps;
};

#endif