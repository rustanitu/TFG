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
  ClusterSet();

  static bool MergeSets(ClusterSet& clusterset);
  static bool MergeSets(std::forward_list<Cluster*>& collection);

  std::forward_list<Cluster*> TakeClosestClusters();

  void Insert(Cluster* cluster);

  void Insert(std::forward_list<Cluster*> list, int n);

  double GetMin()
  {
    return m_min;
  }

  int GetSize()
  {
    return m_size;
  }

  const std::forward_list<Cluster*>& GetClusters()
  {
    return m_clusters;
  }

private:
  void DFS(int e, Cluster** set);

private:
  double m_min;
  int m_size;
  std::vector<bool> m_visited;
  std::vector<Cluster*> m_nodes;
  arma::Mat<double>* m_matrix;
  std::forward_list<Cluster*> m_clusters;
};

#endif