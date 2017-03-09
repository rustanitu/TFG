/// PredictionMap.h
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#ifndef PREDICTION_MAP_H
#define PREDICTION_MAP_H

#include "PMCell.h"
#include "ClusterSet.h"
#include <forward_list>

#define GAUSSIAN_SPREAD 8.0f

template <class T, typename Cell = PMCell<T>>
class PredictionMap
{
public:
  PredictionMap(int coldim, int rowdim);

  ~PredictionMap();

  double SigmaFunc(double sqr_dist);

  bool Init();

  void CleanUp();

  //void PredictWithInverseDistanceWeighting(const double& p, const double& d = 0);

  void Interpolate();
  
  void PredictWithRBF(const std::unordered_set<std::pair<int, int>, SimpleHash>& defined_cells,
    const std::unordered_set<std::pair<int, int>, SimpleHash>& undefined_cells);
  //void PredictWithRBF(arma::Col<double>* w);

  void SetValue(const T& value, const int& i, const int& j);

  T GetValue(const int& i, const int& j);

  bool IsDefined(const int& i, const int& j);

private:
  int m_width;
  int m_height;
  Cell** m_cells;
  std::forward_list<Cluster> m_undefined_clusters;
  //std::forward_list<std::pair<int, int>> m_undefined_cells;
};

#endif