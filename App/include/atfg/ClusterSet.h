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
  ClusterSet(const std::forward_list<Cluster>& collection, int n);

  static void MergeSets(std::forward_list<Cluster>& collection, Cluster& set);

  std::forward_list<Cluster> GetClustersList();

  std::forward_list<Cluster> GetNonMergedClusters();

  double GetMin();

private:
  void DFS(int e, Cluster& set);

private:
  double m_min;
  int m_size;
  std::vector<bool> m_visited;
  std::vector<Cluster*> m_nodes;
  arma::Mat<double> m_matrix;
};

#endif