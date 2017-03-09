/// ClusterSet.cpp
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#include "ClusterSet.h"

#include <armadillo>
#include <vector>
#include <forward_list>

ClusterSet::ClusterSet(const std::forward_list<Cluster>& collection, int n)
: m_min(DBL_MAX)
, m_size(n)
, m_matrix(n, n)
{
  int i = 0;
  for (auto it = collection.begin(); it != collection.end(); ++it, ++i)
  {
    m_nodes.push_back(new Cluster(*it));
    const auto& cellsi = it->GetCells();
    int j = i;
    for (auto itt = it; itt != collection.end(); ++itt, ++j)
    {
      if (i == j)
      {
        m_matrix(i, j) = 0.0f;
        continue;
      }

      double min = DBL_MAX;
      const auto& cellsj = itt->GetCells();
      for (auto cell_i = cellsi.begin(); cell_i != cellsi.end(); ++cell_i)
      {
        for (auto cell_j = cellsj.begin(); cell_j != cellsj.end(); ++cell_j)
        {
          int xi = cell_i->first;
          int yi = cell_i->second;
          int xj = cell_j->first;
          int yj = cell_j->second;
          // Square Distance
          double sqrdist = (xi - xj) * (xi - xj) + (yi - yj) * (yi - yj);
          min = fmin(min, sqrdist);
        }
      }
      m_matrix(i, j) = m_matrix(j, i) = min;
      if (min > SQRT2 + DBL_EPSILON)
        m_min = fmin(m_min, min);
    }
  }

  m_visited = std::vector<bool>(n, false);
}

void ClusterSet::MergeSets(std::forward_list<Cluster>& collection, Cluster& set)
{
  int i = 0;
  const auto& cellsj = set.GetCells();
  for (auto cell_i = collection.begin(); cell_i != collection.end(); ++cell_i, ++i)
  {
    const auto& cellsi = cell_i->GetCells();
    double min = DBL_MAX;
    for (auto cell_i = cellsi.begin(); cell_i != cellsi.end(); ++cell_i)
    {
      for (auto cell_j = cellsi.begin(); cell_j != cellsi.end(); ++cell_j)
      {
        int xi = cell_i->first;
        int yi = cell_i->second;
        int xj = cell_j->first;
        int yj = cell_j->second;
        // Square Distance
        double sqrdist = (xi - xj) * (xi - xj) + (yi - yj) * (yi - yj);
        min = fmin(min, sqrdist);
      }
    }
    if (min > SQRT2 + DBL_EPSILON)
    {
      cell_i->Merge(set);
      return;
    }
  }
  collection.push_front(set);
}

std::forward_list<Cluster> ClusterSet::GetClustersList()
{
  std::forward_list<Cluster> clusterSet(1);
  for (int e = 0; e < m_size; ++e)
  {
    if (!m_visited[e])
    {
      if (!clusterSet.front().IsEmpty())
        clusterSet.push_front(Cluster());
      DFS(e, clusterSet.front());
    }
  }

  if (clusterSet.front().IsEmpty())
    clusterSet.pop_front();

  return clusterSet;
}

std::forward_list<Cluster> ClusterSet::GetNonMergedClusters()
{
  std::forward_list<Cluster> lst;
  for (int i = 0; i < m_nodes.size(); ++i)
  {
    if (m_nodes[i])
    {
      lst.push_front(*m_nodes[i]);
    }
  }
  m_nodes.clear();

  return lst;
}

double ClusterSet::GetMin()
{
  return m_min;
}

void ClusterSet::DFS(int e, Cluster& set)
{
  m_visited[e] = true;
  for (int i = 0; i < m_size; ++i)
  {
    if (m_matrix(e, i) <= m_min + DBL_EPSILON && !m_visited[i])
    {
      if (m_nodes[e])
      {
        set.Merge((*m_nodes[e]));
        delete m_nodes[e];
        m_nodes[e] = NULL;
      }
      if (m_nodes[i])
      {
        set.Merge((*m_nodes[i]));
        delete m_nodes[i];
        m_nodes[i] = NULL;
      }
      DFS(i, set);
    }
  }
}