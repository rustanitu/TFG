/// Cluster.cpp
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#include "Cluster.h"
#include <unordered_set>

Cluster::Cluster()
{

}

void Cluster::Insert(int x, int y)
{
  m_cluster.insert(std::make_pair(x, y));
}

void Cluster::Insert(const std::pair<int, int>& pair)
{
  m_cluster.insert(pair);
}

void Cluster::Merge(const Cluster& cluster)
{
  auto list = cluster.GetCells();
  for (auto it = list.begin(); it != list.end(); ++it)
    Insert(*it);
}

bool Cluster::Has(int x, int y)
{
  auto it = m_cluster.find(std::make_pair(x, y));
  if (it != m_cluster.end())
    return true;
    
  return false;
}

bool Cluster::Has(const std::pair<int, int>& pair)
{
  auto it = m_cluster.find(pair);
  if (it != m_cluster.end())
    return true;

  return false;
}

bool Cluster::IsEmpty()
{
  return m_cluster.empty();
}

const std::unordered_set<std::pair<int, int>, SimpleHash>& Cluster::GetCells() const
{
  return m_cluster;
}

std::unordered_set<std::pair<int, int>, SimpleHash> Cluster::GetNeighborCells(double dist) const
{
  std::unordered_set<std::pair<int, int>, SimpleHash> undefined_cells;

  for (auto it = m_cluster.begin(); it != m_cluster.end(); ++it)
  {
    const std::pair<int, int> pair = *it;
    int xi = pair.first;
    int yi = pair.second;

    for (auto itt = it; itt != m_cluster.end(); ++itt)
    {
      const std::pair<int, int> pair = *itt;
      int xj = pair.first;
      int yj = pair.second;

      // Square Distance
      double sqrdist = (xi - xj) * (xi - xj) + (yi - yj) * (yi - yj);
      dist = fmax(dist, sqrdist);
    }
  }

  for (auto it = m_cluster.begin(); it != m_cluster.end(); ++it)
  {
    const std::pair<int, int> pair = *it;
    int x = pair.first;
    int y = pair.second;

    int radius = sqrt(dist * 0.5f) + 0.5f;

    for (int i = x - radius; i <= x + radius; ++i)
    {
      for (int j = y - radius; j <= y + radius; ++j)
      {
        if (i >= 0 && i <= 255 && j >= 0 && j <= 255)
        {
          double sqr_dist = (i - x) * (i - x) + (y - j) * (y - j);
          if (sqr_dist <= radius)
          {
            const std::pair<int, int> cell = std::make_pair(i, j);
            undefined_cells.insert(cell);
          }
        }
      }
    }
  }

  return undefined_cells;
}