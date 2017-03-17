/// Cluster.h
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#ifndef CLUSTER_H
#define CLUSTER_H

#include "PMCell.h"
#include <forward_list>

class Cluster
{
public:
	Cluster(const int& x, const int& y);

	void InsertDefinedCells(std::forward_list<PMCell*>& cells, std::forward_list<PMCell*>::iterator& it);
  void InsertUndefinedCells(std::forward_list<PMCell*>& cells, std::forward_list<PMCell*>::iterator& it);
	void ReCenter();
	void RemoveDefinedCells(std::forward_list<PMCell*>& cells);
  void RemoveUndefinedCells(std::forward_list<PMCell*>& cells);

	int GetX() const
	{
		return m_x;
	}

	int GetY() const
	{
		return m_y;
	}

	float GetMaxDistance() const
	{
		return m_maxdist;
	}

private:
	int m_x;
	int m_y;
	float m_maxdist;
  std::forward_list<PMCell*> m_defined_cells;
  std::forward_list<PMCell*> m_undefined_cells;
};

#endif