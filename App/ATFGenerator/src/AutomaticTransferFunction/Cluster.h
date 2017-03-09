/// Cluster.h
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#ifndef CLUSTER_H
#define CLUSTER_H

#include <unordered_set>

struct SimpleHash
{
  size_t operator()(const std::pair<int, int>& p) const {
    return p.first + p.second * 256;
  }
};

class Cluster
{
public:
  Cluster();

  void Insert(int x, int y);

  void Insert(const std::pair<int, int>& pair);

  void Merge(const Cluster& cluster);

  bool Has(int x, int y);

  bool Has(const std::pair<int, int>& pair);

  bool IsEmpty();

  const std::unordered_set<std::pair<int, int>, SimpleHash>& GetCells() const;

  std::unordered_set<std::pair<int, int>, SimpleHash> GetNeighborCells(double dist) const;

private:
  std::unordered_set<std::pair<int, int>, SimpleHash> m_cluster;
};

#endif