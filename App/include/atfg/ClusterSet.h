/// ClusterSet.h
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#ifndef CLUSTER_SET_H
#define CLUSTER_SET_H

#include "Cluster.h"

#include <armadillo>
#include <vector>
#include <forward_list>

#define SQRT2 1.41421356237

class ClusterSet
{
public:
	ClusterSet(const int& width, const int& height);

  std::forward_list<Cluster>& KMeans(const int& k, std::forward_list<PMCell*>& defined_cells, std::forward_list<PMCell*>& undefined_cells);
  std::forward_list<Cluster>& KMeans(std::forward_list<PMCell*>& defined_cells, std::forward_list<PMCell*>& undefined_cells);

private:
	void AssignDefinedCellsToClusters(std::forward_list<PMCell*>& cells);
  void AssignUndefinedCellsToClusters(std::forward_list<PMCell*>& cells);

private:
	int m_width;
	int m_height;
  std::forward_list<Cluster> m_clusters;
};

#endif