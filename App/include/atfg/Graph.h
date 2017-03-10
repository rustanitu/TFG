/// Graph.h
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#ifndef GRAPH_H
#define GRAPH_H

#include <armadillo>
#include <vector>

template <class T>
class Graph
{
public:
  Graph()
  {

  }

  template <class C>
  void Init(const C<std::pair<int, int>>& collection)
  {
    double min = DBL_MAX;
    int i = 0;
    for (auto it = collection.begin(); it != collection.end(); ++it, ++i)
    {
      m_nodes.push_back(*it);
      int j = i;
      for (auto itt = it; itt != collection.begin(); ++itt, ++j)
      {
        if (i == j)
        {
          m_matrix(i, j) = 0.0f;
          continue;
        }

        int xi = it->first;
        int yi = it->second;
        int xj = itt->first;
        int yj = itt->second;
        // Square Distance
        m_matrix(i, j) = (xi - xj) * (xi - xj) + (yi - yj) * (yi - yj);
      }
    }
  }

private:
  std::vector<int, T> m_nodes;
  arma::Mat<double> m_matrix;
};

#endif