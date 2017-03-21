/// Cluster.cpp
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#include "Cluster.h"
#include <unordered_set>

Cluster::Cluster(const int& x, const int& y)
  : m_x(x), m_y(y), m_maxdist(0), m_defined_cells_size(0), m_undefined_cells_size(0), m_outdated(false)
{
}

void Cluster::InsertDefinedCells(std::forward_list<PMCell*>& cells, std::forward_list<PMCell*>::iterator& it)
{
	m_defined_cells.splice_after(m_defined_cells.before_begin(), cells, it);
  m_defined_cells_size++;
  m_outdated = true;
}

void Cluster::InsertUndefinedCells(std::forward_list<PMCell*>& cells, std::forward_list<PMCell*>::iterator& it)
{
  m_undefined_cells.splice_after(m_undefined_cells.before_begin(), cells, it);
  m_undefined_cells_size++;
}

void Cluster::ReCenter()
{
  if (!m_outdated)
    return;

	float maxdist = 0;
	float x = 0;
	float y = 0;
	int count = 0;
	for ( auto it = m_defined_cells.begin(); it != m_defined_cells.end(); ++it )
	{
    PMCell* cell = *it;
		x += cell->GetX();
		y += cell->GetY();
		float dist = sqrt((x - m_x) * (x - m_x) + (y - m_y) * (y - m_y));
		maxdist = fmax(maxdist, dist);
		count++;
	}

	m_maxdist = maxdist;

	if ( count > 0 )
	{
		m_x = x / (float)count;
		m_y = y / (float)count;
	}

  m_outdated = false;
}

void Cluster::RemoveDefinedCells(std::forward_list<PMCell*>& cells)
{
	cells.splice_after(cells.before_begin(), m_defined_cells);
  m_defined_cells_size = 0;
}

void Cluster::RemoveUndefinedCells(std::forward_list<PMCell*>& cells)
{
  cells.splice_after(cells.before_begin(), m_undefined_cells);
  m_undefined_cells_size = 0;
}