/// PredictionMap.cpp
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#include "PredictionMap.h"
#include <armadillo>

PredictionMap::PredictionMap(int coldim, int rowdim)
: m_width(coldim)
, m_height(rowdim)
{
}

PredictionMap::~PredictionMap()
{
  CleanUp();
}

double PredictionMap::SigmaFunc(double sqr_dist)
{
  //return sqrt(sqr_dist);
  //return sqr_dist * log(sqrt(sqr_dist) + E);
  double s = GAUSSIAN_SPREAD / 3.0f;
  return exp(-sqr_dist / (s * s)) + 1;
}

bool PredictionMap::Init()
{
  if (m_width < 1 || m_height < 1)
    return false;

  for (int i = 0; i < m_width; ++i)
  {
    for (int j = 0; j < m_height; ++j)
    {
      m_cells_set.insert(new PMCell(i, j, m_width));
    }
  }

  return true;
}

void PredictionMap::CleanUp()
{
  m_cells_set.clear();
}

void PredictionMap::PredictWithInverseDistanceWeighting(const std::forward_list<PMCell*>& defined_cells,
  const std::forward_list<PMCell*>& undefined_cells, const double& p, const double& d)
{
  for (auto it = undefined_cells.begin(); it != undefined_cells.end(); ++it)
  {
    PMCell* undef_cell = *it;
    int xi = undef_cell->GetX();
    int yi = undef_cell->GetY();

    double val = 0;
    double wij = 0;
    for (auto itt = defined_cells.begin(); itt != defined_cells.end(); ++itt)
    {
      PMCell* def_cell = *itt;
      int xj = def_cell->GetX();
      int yj = def_cell->GetY();

      double distance = sqrt(((xi - xj)*(xi - xj)) + ((yi - yj)*(yi - yj)));
      if (d > 0 && distance > d)
        continue;
      double w = (double)1.0f / pow(distance, p);
      val += (w * def_cell->GetValue());
      wij += w;
    }

    if (wij > 0)
      undef_cell->SetValue(val / wij);
  }
}

void PredictionMap::Interpolate()
{
  while(ClusterSet::MergeSets(m_clusterset));

  int t = 0;
  int stop;
  printf("\nSteps of interpolation: ");
  scanf_s("%d", &stop);
  while (t++ < stop)
  //while (!m_undefined_clusters.empty())
  {
    std::forward_list<Cluster*> defined_clusters = m_clusterset.TakeClosestClusters();
    int nclusters = 0;
    for (auto cluster_it = defined_clusters.begin(); cluster_it != defined_clusters.end(); ++cluster_it)
    {
      auto cluster = *cluster_it;
      ++nclusters;

      const auto& defined_cells = cluster->GetCellsList();
      auto undefined_cells = GetNeighborCells(cluster, m_clusterset.GetMin());

      //for (auto cell = defined_cells.begin(); cell != defined_cells.end(); ++cell)
      //{
      //  (*cell)->SetValue(1.0f);
      //}
      //  
      //for (auto cell = undefined_cells.begin(); cell != undefined_cells.end(); ++cell)
      //{
      //  (*cell)->SetValue(0.0f);
      //}

      //PredictWithRBF(defined_cells, undefined_cells);

      for (auto cell = undefined_cells.begin(); cell != undefined_cells.end(); ++cell)
      {
        cluster->Insert(*cell);
      }
    }
    
    m_clusterset.Insert(defined_clusters, nclusters);
    //if (m_clusterset.GetSize() == nclusters)
    //  break;
  }

  //return;
  
  std::forward_list<PMCell*> defined_cells;
  std::forward_list<PMCell*> undefined_cells;
  const std::forward_list<Cluster*>& clusters = m_clusterset.GetClusters();
  for (auto it = clusters.begin(); it != clusters.end(); ++it)
  {
    Cluster* cluster = *it;
    std::forward_list<PMCell*> list = cluster->GetCellsList();

    while (!list.empty())
    {
      PMCell* cell = list.front();
      if (cell->IsDefined())
      {
        defined_cells.splice_after(defined_cells.before_begin(), list, list.before_begin());
      }
      else
      {
        undefined_cells.splice_after(undefined_cells.before_begin(), list, list.before_begin());
      }
    }
    PredictWithRBF(defined_cells, undefined_cells);
  }
}
  
void PredictionMap::PredictWithRBF(const std::forward_list<PMCell*>& defined_cells,
  const std::forward_list<PMCell*>& undefined_cells)
{
  const int n = std::distance(defined_cells.begin(), defined_cells.end());
  arma::Mat<double> sigma_mat(n, n);
  arma::Col<double> p(n);

  int i = 0;
  for (auto row = defined_cells.begin(); row != defined_cells.end(); ++row, ++i)
  {
    p[i] = (*row)->GetValue();

    int j = i;
    for (auto col = row; col != defined_cells.end(); ++col, ++j)
    {
      if (i == j)
      {
        sigma_mat(i, j) = 0.0f;
        continue;
      }

      int xi = (*row)->GetX();
      int yi = (*row)->GetY();
      int xj = (*col)->GetX();
      int yj = (*col)->GetY();

      double sqr_dist = (xi - xj) * (xi - xj) + (yi - yj) * (yi - yj);
      sigma_mat(i, j) = sigma_mat(j, i) = SigmaFunc(sqr_dist);
    }
  }

  arma::Mat<double> inverse_sigma_mat = arma::inv(sigma_mat);
  arma::Col<double> w = inverse_sigma_mat * p;

  for (auto row = undefined_cells.begin(); row != undefined_cells.end(); ++row)
  {
    int x = (*row)->GetX();
    int y = (*row)->GetY();

    arma::Row<double> sigma(n);
    int count = 0;
    for (auto col = defined_cells.begin(); col != defined_cells.end(); ++col)
    {
      int xi = (*col)->GetX();
      int yi = (*col)->GetY();

      double sqr_dist = (x - xi) * (x - xi) + (y - yi) * (y - yi);
      sigma(count++) = SigmaFunc(sqr_dist);
    }

    (*row)->SetValue(arma::dot(sigma, w));
    //cluster.Insert(x, y);
  }
}

std::forward_list<PMCell*> PredictionMap::GetNeighborCells(Cluster* cluster, double dist) const
{
  const std::forward_list<PMCell*> list = cluster->GetCellsList();
  std::forward_list<PMCell*> undefined_cells;

  //for (auto it = set.begin(); it != set.end(); ++it)
  //{
  //  int xi = (*it)->GetX();
  //  int yi = (*it)->GetY();

  //  for (auto itt = it; itt != set.end(); ++itt)
  //  {
  //    int xj = (*it)->GetX();
  //    int yj = (*it)->GetY();

  //    // Square Distance
  //    double sqrdist = (xi - xj) * (xi - xj) + (yi - yj) * (yi - yj);
  //    dist = fmax(dist, sqrdist);
  //  }
  //}
 
  for (auto it = list.begin(); it != list.end(); ++it)
  {
    int x = (*it)->GetX();
    int y = (*it)->GetY();

    double radius = sqrt(dist) * 0.5f + 0.5f;

    for (int i = x - radius; i <= x + radius; ++i)
    {
      for (int j = y - radius; j <= y + radius; ++j)
      {
        if (i >= 0 && i <= 255 && j >= 0 && j <= 255)
        {
          PMCell* cell = GetCell(i, j);
          double sqr_dist = (i - x) * (i - x) + (y - j) * (y - j);
          if (sqrt(sqr_dist) <= radius && !cell->IsDefined())
          {
            undefined_cells.push_front(cell);
          }
        }
      }
    }
  }

  return undefined_cells;
}

//void PredictionMap::PredictWithRBF(arma::Col<double>* w);
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

void PredictionMap::SetValue(const double& value, const int& i, const int& j)
{
  if (i > m_width || i > m_height || i < 0 || j < 0)
    return;

  PMCell* cell = GetCell(i, j);
  cell->SetValue(value);
  Cluster* cluster = new Cluster(m_width);
  cluster->Insert(cell);
  m_clusterset.Insert(cluster);
}

double PredictionMap::GetValue(const int& i, const int& j)
{
  return GetCell(i, j)->GetValue();
}

bool PredictionMap::IsDefined(const int& i, const int& j)
{
  return GetCell(i, j)->IsDefined();
}

PMCell* PredictionMap::GetCell(const int& i, const int& j) const
{
  PMCell* dummy_cell = new PMCell(i, j, m_width);
  auto it = m_cells_set.find(dummy_cell);
  delete dummy_cell;
  if (it != m_cells_set.end())
  {
    return *it;
  }

  return NULL;
}