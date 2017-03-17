/// PredictionMap.cpp
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#include "PredictionMap.h"
#include <armadillo>

PredictionMap::PredictionMap(int coldim, int rowdim)
: m_width(coldim)
, m_height(rowdim)
{
	for ( int i = 0; i < m_width; ++i )
	{
		for ( int j = 0; j < m_height; ++j )
		{
      m_map.insert(std::pair<int, PMCell*>(GetKey(i, j), new PMCell(i, j)));
		}
	}
}

PredictionMap::~PredictionMap()
{
}

double PredictionMap::SigmaFunc(double sqr_dist)
{
  //return sqrt(sqr_dist);
  return sqr_dist * log(sqrt(sqr_dist) + E);
  double s = GAUSSIAN_SPREAD / 3.0f;
  return exp(-sqr_dist / (s * s)) + 1;
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
  int nclusters;
  std::cout << std::endl;
  std::cout << "Cluster per dimension: ";
  std::cin >> nclusters;
  std::cout << std::endl;

  if (nclusters == 0)
    return;

  std::forward_list<PMCell*> all_undefined_cells;
  for (auto it = m_map.begin(); it != m_map.end(); ++it)
  {
    if (!it->second->IsDefined())
      all_undefined_cells.push_front(it->second);
  }

	ClusterSet clusterset(m_width, m_height);
  std::forward_list<Cluster> clusters = clusterset.KMeans(nclusters, m_cells, all_undefined_cells);

  float val = 0.0f;
	for ( auto cluster_it = clusters.begin(); cluster_it != clusters.end(); ++cluster_it )
	{
		std::forward_list<PMCell*> defined_cells;
    std::forward_list<PMCell*> undefined_cells;
    cluster_it->RemoveDefinedCells(defined_cells);
    cluster_it->RemoveUndefinedCells(undefined_cells);

#if 1
		PredictWithRBF(defined_cells, undefined_cells);
#else
    val += 1.0f;
    if (!defined_cells.empty())
    {
      defined_cells.splice_after(defined_cells.before_begin(), undefined_cells);
      for (auto it = defined_cells.begin(); it != defined_cells.end(); ++it)
      {
        (*it)->SetValue(val);
      }

      int cx = cluster_it->GetX();
      int cy = cluster_it->GetY();
      m_map.at(GetKey(cx, cy))->SetValue(0.0f);
    }
#endif

    defined_cells.clear();
    undefined_cells.clear();
	}
}
  
void PredictionMap::PredictWithRBF(const std::forward_list<PMCell*>& defined_cells,
  std::forward_list<PMCell*>& undefined_cells)
{
  if (defined_cells.empty())
    return;

  const int n = std::distance(defined_cells.begin(), defined_cells.end());
  arma::Mat<double> sigma_mat(n, n);
  arma::Col<double> p(n);

  int i = 0;
  for (auto row = defined_cells.begin(); row != defined_cells.end(); ++row, ++i)
  {
    PMCell* celli = *row;
    p[i] = celli->GetValue();

    int j = i;
    for (auto col = row; col != defined_cells.end(); ++col, ++j)
    {
      if (i == j)
      {
        sigma_mat(i, j) = 0.0f;
        continue;
      }

      PMCell* cellj = *col;
      int xi = celli->GetX();
      int yi = celli->GetY();
      int xj = cellj->GetX();
      int yj = cellj->GetY();

      double sqr_dist = (xi - xj) * (xi - xj) + (yi - yj) * (yi - yj);
      sigma_mat(i, j) = sigma_mat(j, i) = SigmaFunc(sqr_dist);
    }
  }

  arma::Mat<double> inverse_sigma_mat = arma::inv(sigma_mat);
  arma::Col<double> w = inverse_sigma_mat * p;

  for (auto row = undefined_cells.begin(); row != undefined_cells.end(); ++row)
  {
    PMCell* celli = *row;
    int x = celli->GetX();
    int y = celli->GetY();

    arma::Row<double> sigma(n);
    int count = 0;
    for (auto col = defined_cells.begin(); col != defined_cells.end(); ++col)
    {
      PMCell* cellj = *col;
      int xi = cellj->GetX();
      int yi = cellj->GetY();

      double sqr_dist = (x - xi) * (x - xi) + (y - yi) * (y - yi);
      sigma(count++) = SigmaFunc(sqr_dist);
    }

		double val = arma::dot(sigma, w);
    celli->SetValue(val);
  }
}

void PredictionMap::SetValue(const double& value, const int& i, const int& j)
{
  PMCell* cell = m_map.at(GetKey(i, j));
  cell->SetValue(value);
  m_cells.push_front(cell);
}

std::forward_list<PMCell*> PredictionMap::GetNeighborCells(const Cluster& cluster) const
{
	int cx = cluster.GetX();
	int cy = cluster.GetY();
	float maxdist = cluster.GetMaxDistance();

	std::forward_list<PMCell*> undefined_cells;
	for ( int i = cx - maxdist; i <= cx + maxdist; ++i )
	{
		for ( int j = cy - maxdist; j <= cy + maxdist; ++j )
		{
			if ( i >= 0 && i <= 255 && j >= 0 && j <= 255 )
			{
        PMCell* cell = m_map.find(GetKey(i, j))->second;
				double sqr_dist = (i - cx) * (i - cx) + (cy - j) * (cy - j);
        if (sqrt(sqr_dist) <= maxdist && !cell->IsDefined())
				{
					undefined_cells.push_front(cell);
				}
			}
		}
	}

	return undefined_cells;
}