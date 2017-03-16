/// ClusterSet.cpp
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#include "ClusterSet.h"

#include <armadillo>
#include <vector>
#include <forward_list>

ClusterSet::ClusterSet()
: m_min(DBL_MAX)
, m_matrix(NULL)
, m_size(0)
{
}

bool ClusterSet::MergeSets(ClusterSet& clusterset)
{
  if (MergeSets(clusterset.m_clusters))
  {
    clusterset.m_size--;
    return true;
  }
  return false;
}

bool ClusterSet::MergeSets(std::forward_list<Cluster*>& collection)
{
  auto before_cluster1 = collection.before_begin();
  for (auto cluster1 = collection.begin(); cluster1 != collection.end(); ++cluster1)
  {
    const auto& cells_cluster1 = (*cluster1)->GetCells();
    for (auto cell1 = cells_cluster1.begin(); cell1 != cells_cluster1.end(); ++cell1)
    {
      auto before_cluster2 = cluster1;
      for (auto cluster2 = std::next(cluster1); cluster2 != collection.end(); ++cluster2)
      {
        const auto& cells_cluster2 = (*cluster2)->GetCells();
        for (auto cell2 = cells_cluster2.begin(); cell2 != cells_cluster2.end(); ++cell2)
        {
          int xi = (*cell1)->GetX();
          int yi = (*cell1)->GetY();
          int xj = (*cell2)->GetX();
          int yj = (*cell2)->GetY();

          // Square Distance
          double sqrdist = (xi - xj) * (xi - xj) + (yi - yj) * (yi - yj);
          if (sqrdist <= SQRT2)
          {
            Cluster* c1 = *cluster1;
            Cluster* c2 = *cluster2;
            collection.erase_after(before_cluster2);
            collection.erase_after(before_cluster1);
            collection.push_front(Cluster::Merge(&c1, &c2));
            return true;
          }
        }
        before_cluster2 = cluster2;
      }
    }
    before_cluster1 = cluster1;
  }

  return false;
}

std::forward_list<Cluster*> ClusterSet::TakeClosestClusters()
{
  if (m_size == 0)
    return std::forward_list<Cluster*>();

  if (m_size == 1)
  {
    std::forward_list<Cluster*> lst;
    lst.splice_after(lst.before_begin(), m_clusters);
    m_size = 0;
    return lst;
  }
  
  m_min = DBL_MAX;
  delete m_matrix;
  m_matrix = new arma::Mat<double>(m_size, m_size);

  int i = 0;
  for (auto it = m_clusters.begin(); it != m_clusters.end(); ++it, ++i)
  {
    m_nodes.push_back(*it);
    const auto& cellsi = (*it)->GetCells();
    int j = i;
    for (auto itt = it; itt != m_clusters.end(); ++itt, ++j)
    {
      if (i == j)
      {
        m_matrix->at(i, j) = 0.0f;
        continue;
      }

      double min = DBL_MAX;
      const auto& cellsj = (*itt)->GetCells();
      for (auto cell_i = cellsi.begin(); cell_i != cellsi.end(); ++cell_i)
      {
        for (auto cell_j = cellsj.begin(); cell_j != cellsj.end(); ++cell_j)
        {
          int xi = (*cell_i)->GetX();
          int yi = (*cell_i)->GetY();
          int xj = (*cell_j)->GetX();
          int yj = (*cell_j)->GetY();
          // Square Distance
          double sqrdist = (xi - xj) * (xi - xj) + (yi - yj) * (yi - yj);
          min = fmin(min, sqrdist);
        }
      }
      m_matrix->at(i, j) = m_matrix->at(j, i) = min;
      m_min = fmin(m_min, min);
    }
  }

  m_visited = std::vector<bool>(m_size, false);

  std::forward_list<Cluster*> list;
  for (int e = 0; e < m_size; ++e)
  {
    if (!m_visited[e])
    {
      Cluster* cluster = NULL;
      DFS(e, &cluster);
      if (cluster)
        list.push_front(cluster);
    }
  }
  m_visited.clear();

  m_clusters.clear();
  m_size = 0;
  for (int i = 0; i < m_nodes.size(); ++i)
  {
    if (m_nodes[i])
    {
      m_clusters.push_front(m_nodes[i]);
      m_size++;
    }
  }
  m_nodes.clear();

  return list;
}

void ClusterSet::DFS(int e, Cluster** set)
{
  m_visited[e] = true;
  for (int i = 0; i < m_size; ++i)
  {
    if (m_matrix->at(e, i) <= m_min + DBL_EPSILON && !m_visited[i])
    {
      if (m_nodes[e])
      {
        *set = Cluster::Merge(set, &m_nodes[e]);
        m_nodes[e] = NULL;
      }
      if (m_nodes[i])
      {
        *set = Cluster::Merge(set, &m_nodes[i]);
        m_nodes[i] = NULL;
      }

      DFS(i, set);
    }
  }
}

void ClusterSet::Insert(Cluster* cluster)
{
  m_clusters.push_front(cluster);
  m_size++;
}

void ClusterSet::Insert(std::forward_list<Cluster*> list, int n)
{
  list.splice_after(list.before_begin(), m_clusters);
  m_clusters.splice_after(m_clusters.before_begin(), list);
  m_size += n;
}