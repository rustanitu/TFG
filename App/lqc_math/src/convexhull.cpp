﻿#include <math/convexhull.h>
#include <iostream>
#include <math/MUtils.h>

namespace lqc
{
  ConvexHull2D::ConvexHull2D ()
  {}

  ConvexHull2D::~ConvexHull2D ()
  {}

  std::vector<Vector2f> ConvexHull2D::GetCCWConvexPoints ()
  {
    return m_ccw_convex_points;
  }

  namespace ch2d
  {
    bool lexicografic_compare (Vector2f a, Vector2f b)
    {
      if (a.x == b.x)
        return a.y < b.y;
      else
        return a.x < b.x;
    }

    Incremental::Incremental ()
    {}

    Incremental::~Incremental ()
    {}

    void Incremental::Do (std::vector<Vector2f> points)
    {
      std::vector<Vector2f> CH;

      if ((int)points.size () < 3)
      {
        m_ccw_convex_points = CH;
        return;
      }
      //lqc_ch_lexHeap::lexicografic_heapsort(points);
      std::sort (points.begin (), points.end (), lexicografic_compare);

      //Triangle p1 p2 p3
      CH.push_back (points[0]);
      CH.push_back (points[1]);
      CH.push_back (points[2]);

      int orientation = Orientation (CH[1] - CH[0], CH[2] - CH[0]);
      if (orientation == 1)
      {
        Vector2f aux = CH[2];
        CH[2] = CH[1];
        CH[1] = aux;
      }

      for (int i = 3; i < (int)points.size (); i++)
      {
        float rightmost = CH[0].x;
        int j = 0;
        for (int c = 0; c < (int)CH.size (); c++)
        {
          if (rightmost < CH[c].x)
          {
            j = c;
            rightmost = CH[c].x;
          }
        }

        int upperTangent = j;
        int lowerTangent = j;
        for (int k = 0; k < (int)CH.size (); k++)
        {
          Vector2f aux_point = points[i];
          if (upperTangent != k)
          {
            int aux_orientation = Orientation (CH[upperTangent] - points[i], CH[k] - points[i]);
            if (aux_orientation == 1)
            {
              upperTangent = k;
            }
          }
          if (lowerTangent != k)
          {
            int aux_orientation = Orientation (CH[lowerTangent] - points[i], CH[k] - points[i]);
            if (aux_orientation == 2)
            {
              lowerTangent = k;
            }
          }
        }
        if (lowerTangent < upperTangent)
        {
          CH.erase (CH.begin () + lowerTangent + 1, CH.begin () + upperTangent);
          CH.insert (CH.begin () + lowerTangent + 1, points[i]);
        }
        else if (lowerTangent == upperTangent)
        {
          CH.insert (CH.begin () + lowerTangent + 1, points[i]);
        }
        else
        {
          CH.erase (CH.begin () + lowerTangent + 1, CH.end ());
          CH.push_back (points[i]);
          CH.erase (CH.begin (), CH.begin () + upperTangent);
        }
      }
      CH.push_back (points[0]);
      m_ccw_convex_points = CH;
    }
  }
}

