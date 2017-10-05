/// Cluster.h
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#ifndef CLUSTER_H
#define CLUSTER_H

#include "PMCell.h"
#include <unordered_set>
#include <forward_list>

class Cluster
{
public:
  Cluster(const int& base);

  void Insert(PMCell* cell);

  bool Has(const int& i, const int& j);

  bool Has(PMCell* cell);

  bool IsEmpty();

  const std::unordered_set<PMCell*, PMCellHash, PMCellComparator>& GetCells() const;
  std::forward_list<PMCell*>& GetCellsList();
  
  double GetValue(const int& i, const int& j);
  
  bool IsDefined(const int& i, const int& j);
  
  static Cluster* Merge(Cluster** cluster1, Cluster** cluster2);

private:
  PMCell* GetCell(const int& i, const int& j);

private:
  int m_base;
  int m_size;
  std::forward_list<PMCell*> m_list;
  std::unordered_set<PMCell*, PMCellHash, PMCellComparator> m_set;
};

#endif