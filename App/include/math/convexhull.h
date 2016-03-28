#ifndef LQC_MATH_CONVEX_HULL_H
#define LQC_MATH_CONVEX_HULL_H

#include <cmath>
#include <vector>
#include <stack>
#include <iostream>
#include <cstdlib>
#include <algorithm>    // std::sort
#include <math/MGeometry.h>
#include <math/Matrix4.h>
#include <math/Vector2.h>
/**
* Order of output vertices: Counter Clockwise
*
*     8                       7
*      \                     /
*       *------------------*
*       | *    *    *     *  \
*       |    *  *  *    *  *  \
*   9 - *  *    *    *  *   *  * - 6
*      / *    *    *    *    * |
* 1 - *  *    *  *  *  *   *   * - 5
*     |    *        *    *    /
*     | *     *   *  * *  *  /
* 2 - *  *      *     *     /
*      \    *       *    * /
*       \*     *  *   *   /
*        \ *  *    *     /
*         *-------------*
*        /                \
*       3                  4
*
* http://en.wikipedia.org/wiki/Convex_hull_algorithms
**/

namespace lqc
{
  class ConvexHull2D
  {
  public:
    ConvexHull2D ();
    ~ConvexHull2D ();

    std::vector<Vector2f> GetCCWConvexPoints ();

    virtual void Do (std::vector<Vector2f> points) = 0;
  protected:
    std::vector<Vector2f> m_ccw_convex_points;
  };

  namespace ch2d
  {
    class Incremental : ConvexHull2D
    {
    public:
      Incremental ();
      ~Incremental ();

      virtual void Do (std::vector<Vector2f> points);
    };
  }
}

#endif

