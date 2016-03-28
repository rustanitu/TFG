#ifndef LQC_CLUSTERING_MEAN_SHIFT_H
#define LQC_CLUSTERING_MEAN_SHIFT_H

#include <math/Vector2.h>
#include <vector>

namespace lqc
{
  void MeanShift (std::vector<Vector2f>& centroids, std::vector<Vector2f> points, float Bandwidth, float threshold);
}

#endif