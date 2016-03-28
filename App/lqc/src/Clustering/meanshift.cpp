#include <lqc/Clustering/meanshift.h>

#include <math/MGeometry.h>
#include <math/Vector2.h>

#include <vector>

namespace lqc
{
  void MeanShift (std::vector<Vector2f>& centroids, std::vector<Vector2f> points, float Bandwidth, float stopthreshold)
  {
    for (int i = 0 ; i < (int)points.size() ; i++)
    {
      Vector2f p = points[i];
      Vector2f ppast = p;
      do
      {
        float N = 1.0f;
        Vector2f m = Vector2f(points[i].x, points[i].y);
        for (int j = 0 ; j < (int)points.size() ; j++)
        {
          if( i != j)
          {
            if (Distance(points[j], p) < Bandwidth)
            {
              m = m + points[j];
              N = N + 1.0f;
            }
          }
        }
        ppast = p;
        p = m/N;
      } while (Distance(p, ppast) > stopthreshold);
      centroids.push_back(Vector2f(p.x, p.y));
    }
  }
}
