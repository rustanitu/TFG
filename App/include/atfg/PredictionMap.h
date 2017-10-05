/// PredictionMap.h
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#ifndef PREDICTION_MAP_H
#define PREDICTION_MAP_H

#include "PMCell.h"
#include "ClusterSet.h"
#include <forward_list>
#include <unordered_map>

#define GAUSSIAN_SPREAD 16.0f

class PredictionMap
{
public:
  PredictionMap(int coldim, int rowdim);

  ~PredictionMap();

  double SigmaFunc(double sqr_dist);

  void Interpolate();

  void PredictWithInverseDistanceWeighting(const std::forward_list<PMCell*>& defined_cells,
    const std::forward_list<PMCell*>& undefined_cells, const double& p, const double& d = 0);
  
  void PredictWithRBF(const std::forward_list<PMCell*>& defined_cells,
    std::forward_list<PMCell*>& undefined_cells);

	std::forward_list<PMCell*> GetNeighborCells(const Cluster& cluster) const;

  void SetValue(const double& value, const int& i, const int& j);

	double GetValue(const int& i, const int& j)
	{
    return m_map.at(GetKey(i, j))->GetValue();
	}

	const std::forward_list<PMCell*>& GetCells() const
	{
		return m_cells;
	}

  int GetKey(const int& i, const int& j) const
  {
    return i + j * m_width;
  }

private:
  int m_width;
  int m_height;
  std::unordered_map<int, PMCell*> m_map;
	std::forward_list<PMCell*> m_cells;
};

#endif