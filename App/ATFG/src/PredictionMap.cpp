/// PredictionMap.cpp
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#include "PredictionMap.h"
#include <armadillo>

PredictionMap<double, DoubleCell>::PredictionMap(int coldim, int rowdim)
: m_cells(NULL)
, m_width(coldim)
, m_height(rowdim)
{
}

PredictionMap<double, DoubleCell>::~PredictionMap()
{
  CleanUp();
}

double PredictionMap<double, DoubleCell>::SigmaFunc(double sqr_dist)
{
  //return sqrt(sqr_dist);
  return fmax(1.0f, sqr_dist * log(sqrt(sqr_dist)));
  double s = GAUSSIAN_SPREAD / 3.0f;
  return exp(-sqr_dist / (s * s));
}

bool PredictionMap<double, DoubleCell>::Init()
{
  if (m_width < 1 || m_height < 1)
    return false;

  m_cells = new DoubleCell*[m_width];
  if (!m_cells)
    return false;

  int last = -1;
  for (int i = 0; i < m_width; ++i)
  {
    m_cells[i] = new DoubleCell[m_height];
    if (!m_cells[i])
    {
      last = i - 1;
      break;
    }

    //for (int j = 0; j < m_height; ++j)
    //{
    //  m_undefined_cells.push_front(std::make_pair(i, j));
    //}
  }

  if (last > -1)
  {
    for (int i = 0; i < last; ++i)
      delete[] m_cells[i];

    delete[] m_cells;
  }

  return true;
}

template<>
void PredictionMap<double, DoubleCell>::CleanUp()
{
  if (!m_cells)
    return;

  for (int i = 0; i < m_width; ++i)
  {
    for (int j = 0; j < m_height; ++j)
    {
      delete[] m_cells[i];
      m_cells[i] = NULL;
    }
  }

  delete[] m_cells;
  m_cells = NULL;
  m_undefined_clusters.clear();
  //m_undefined_cells.clear();
}

//void PredictionMap<double, DoubleCell>::PredictWithInverseDistanceWeighting(const double& p, const double& d = 0)
//{
//  while (!m_undefined_cells.empty())
//  {
//    const std::pair<int, int>& pair = m_undefined_cells.front();
//    int i = pair.first;
//    int j = pair.second;
//    m_undefined_cells.pop_front();

//    T val = 0;
//    double wij = 0;
//    for (auto it = m_undefined_clusters.begin(); it != m_undefined_clusters.end(); ++it)
//    {
//      int l = it->first;
//      int m = it->second;

//      double distance = sqrt(((i - l)*(i - l)) + ((j - m)*(j - m)));
//      if (d > 0 && distance > d)
//        continue;
//      double w = (double)1.0f / pow(distance, p);
//      val += (w * m_cells[l][m].GetValue());
//      wij += w;
//    }

//    if (wij > 0)
//      m_cells[i][j].SetValue(val / wij);
//  }
//  m_undefined_clusters.clear();
//}

void PredictionMap<double, DoubleCell>::Interpolate()
{
  int t = 0;
  while (t++ < 5)
  //while (!m_undefined_clusters.empty())
  {
    auto first = m_undefined_clusters.begin();
    auto last = m_undefined_clusters.end();
    int n = std::distance(first, last);

    ClusterSet clusterset(m_undefined_clusters, n);
    std::forward_list<Cluster> defined_clusters = clusterset.GetClustersList();
    m_undefined_clusters.clear();
    m_undefined_clusters = clusterset.GetNonMergedClusters();
    for (auto cluster = defined_clusters.begin(); cluster != defined_clusters.end(); ++cluster)
    {
      const std::unordered_set<std::pair<int, int>, SimpleHash>& defined_cells = cluster->GetCells();
      std::unordered_set<std::pair<int, int>, SimpleHash> undefined_cells = cluster->GetNeighborCells(clusterset.GetMin());

      for (auto cell = defined_cells.begin(); cell != defined_cells.end(); ++cell)
      {
        m_cells[cell->first][cell->second].SetValue(1.0f);
        m_cells[cell->first][cell->second].SetDefined(true);
      }
        
      for (auto cell = undefined_cells.begin(); cell != undefined_cells.end(); ++cell)
      {
        if (!m_cells[cell->first][cell->second].IsDefined())
        {
          m_cells[cell->first][cell->second].SetValue(0.0f);
          m_cells[cell->first][cell->second].SetDefined(true);
          cluster->Insert(*cell);
        }
      }

      //PredictWithRBF(defined_cells, undefined_cells);
    }
  }
}
  
void PredictionMap<double, DoubleCell>::PredictWithRBF(const std::unordered_set<std::pair<int, int>, SimpleHash>& defined_cells,
  const std::unordered_set<std::pair<int, int>, SimpleHash>& undefined_cells)
{
  const int n = std::distance(defined_cells.begin(), defined_cells.end());
  arma::Mat<double> sigma_mat(n, n);
  arma::Col<double> p(n);

  int i = 0;
  for (auto row = defined_cells.begin(); row != defined_cells.end(); ++row, ++i)
  {
    p[i] = m_cells[row->first][row->second].GetValue();

    int j = i;
    for (auto col = row; col != defined_cells.end(); ++col, ++j)
    {
      if (i == j)
      {
        sigma_mat(i, j) = 0.0f;
        continue;
      }

      int xi = row->first;
      int yi = row->second;
      int xj = col->first;
      int yj = col->second;

      double sqr_dist = (xi - xj) * (xi - xj) + (yi - yj) * (yi - yj);
      sigma_mat(i, j) = sigma_mat(j, i) = SigmaFunc(sqr_dist);
    }
  }

  arma::Mat<double> inverse_sigma_mat = arma::inv(sigma_mat);
  arma::Col<double> w = inverse_sigma_mat * p;

  for (auto row = undefined_cells.begin(); row != undefined_cells.end(); ++row)
  {
    int x = row->first;
    int y = row->second;

    arma::Row<double> sigma(n);
    int count = 0;
    for (auto col = defined_cells.begin(); col != defined_cells.end(); ++col)
    {
      int xi = col->first;
      int yi = col->second;

      double sqr_dist = (x - xi) * (x - xi) + (y - yi) * (y - yi);
      sigma(count++) = SigmaFunc(sqr_dist);
    }

    m_cells[x][y].SetValue(arma::dot(sigma, w));
    m_cells[x][y].SetDefined(true);
    //cluster.Insert(x, y);
  }
}

//void PredictionMap<double, DoubleCell>::PredictWithRBF(arma::Col<double>* w);
//{
//  const auto first = m_undefined_clusters.begin();
//  const auto last = m_undefined_clusters.end();
//  int n = w->size();
//  
//  while (!m_undefined_cells.empty())
//  {
//    const std::pair<int, int>& pair = m_undefined_cells.front();
//    int i = pair.first;
//    int j = pair.second;
//    m_undefined_cells.pop_front();
//
//    arma::Row<double> sigma(n);
//    int count = 0;
//    for (auto it = first; it != last; ++it)
//    {
//      int x = it->first;
//      int y = it->second;
//
//      double sqr_dist = (x - i) * (x - i) + (y - j) * (y - j);
//      sigma(count++) = SigmaFunc(sqr_dist);
//    }
//
//    m_cells[i][j].SetValue(arma::dot(sigma, (*w)));
//  }
//  m_undefined_clusters.clear();
//}

void PredictionMap<double, DoubleCell>::SetValue(const double& value, const int& i, const int& j)
{
  if (i > m_width || i > m_height || i < 0 || j < 0)
    return;

  m_cells[i][j].SetValue(value);
  m_cells[i][j].SetDefined(true);
  Cluster cluster;
  cluster.Insert(i, j);
  //Cluster::MergeSets(m_undefined_clusters, cluster);
  m_undefined_clusters.push_front(cluster);
  //m_undefined_cells.remove(std::make_pair(i, j));
}

double PredictionMap<double, DoubleCell>::GetValue(const int& i, const int& j)
{
  return m_cells[i][j].GetValue();
}

bool PredictionMap<double, DoubleCell>::IsDefined(const int& i, const int& j)
{
  return m_cells[i][j].IsDefined();
}