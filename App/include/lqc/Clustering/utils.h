#ifndef LQC_CLUSTERING_UTILS_H
#define LQC_CLUSTERING_UTILS_H

#include <math/MGeometry.h>
#include <math/convexhull.h>

#include <math/Vector2.h>
#include <math/Vector3.h>
#include <vector>

namespace lqc
{
  class Cluster2D
  {
  public:
    std::vector<Vector2f> points;
    std::vector<Vector2f> centroids;
    Vector3f m_color;
    std::vector<Vector2f> m_convex_hull_ccw;

    void* data;
  };

  void MakeConvexHullCCW (Cluster2D& gencluster)
  {
    //gencluster.m_convex_hull_ccw = ConvexHull2D::Incremental (gencluster.points);
  }

  std::vector<Cluster2D> GroupCentroids (std::vector<Vector2f> points, std::vector<Vector2f> centroids, float MaxDistanceGrouping)
  {
    std::vector<Cluster2D> clusters;
    printf ("Group centroids\n");
    while (centroids.size () > 0)
    {
      Cluster2D gclust;
      gclust.points.push_back (points[0]);
      gclust.centroids.push_back (centroids[0]);
      points.erase (points.begin ());
      centroids.erase (centroids.begin ());
      int c = 0;
      while (c < (int)gclust.centroids.size ())
      {
        int j = 0;
        while (j < (int)centroids.size ())
        {
          if (Distance (gclust.centroids[c], centroids[j]) <= MaxDistanceGrouping)
          {
            gclust.points.push_back (points[j]);
            gclust.centroids.push_back (centroids[j]);
            points.erase (points.begin () + j);
            centroids.erase (centroids.begin () + j);
          }
          else
            j++;
        }
        c++;
      }
      MakeConvexHullCCW (gclust);
      clusters.push_back (gclust);
    }
    return clusters;
  }
}

#endif