/// PredictionMap.h
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#ifndef PREDICTION_MAP_H
#define PREDICTION_MAP_H

#include "PMCell.h"
#include "ClusterSet.h"
#include <forward_list>

#define GAUSSIAN_SPREAD 4.0f

class PredictionMap
{
public:
  PredictionMap(int coldim, int rowdim);

  ~PredictionMap();

  double SigmaFunc(double sqr_dist);

  bool Init();

  void CleanUp();

  void PredictWithInverseDistanceWeighting(const std::forward_list<PMCell*>& defined_cells,
    const std::forward_list<PMCell*>& undefined_cells, const double& p, const double& d = 0);

  void Interpolate();
  
  void PredictWithRBF(const std::forward_list<PMCell*>& defined_cells,
    const std::forward_list<PMCell*>& undefined_cells);
  //void PredictWithRBF(arma::Col<double>* w);

  std::forward_list<PMCell*> GetNeighborCells(Cluster* cluster, double dist) const;

  void SetValue(const double& value, const int& i, const int& j);

  double GetValue(const int& i, const int& j);
  
  bool IsDefined(const int& i, const int& j);

//private:
  PMCell* GetCell(const int& i, const int& j) const;

private:
  int m_width;
  int m_height;
  std::unordered_set<PMCell*, PMCellHash, PMCellComparator> m_cells_set;
  ClusterSet m_clusterset;
};

#endif