#ifndef LQC_UTILS_FUNCTION_H
#define LQC_UTILS_FUNCTION_H

#include <cstdlib>
#include <cmath>

namespace lqc
{
  class Function1V
  {
  public:
    Function1V ()
    {
      func1V = sin;
    }

    double Call (double x)
    {
      return func1V (x);
    }

    double ComputeSum (double lo, double hi)
    {
      double  sum = 0.0;
      for (int i = 0; i <= 100; i++)
      {
        double  x, y;
        x = i / 100.0 * (hi - lo) + lo;
        y = (*func1V)(x);
        sum += y;
      }
      return (sum / 100.0);
    }

  private:
    double (*func1V) (double x);
  };

  double sin2cos2 (double x, double y)
  {
    return sin (x*x) * cos (y*y);
  }

  class Function2V
  {
  public:
    Function2V ()
    {
      func2V = sin2cos2;
    }

    double Call (double x, double y)
    {
      return func2V (x, y);
    }

  private:
    double (*func2V) (double x, double y);
  };

  double f3v (double x, double y, double z)
  {
    return (x*x) - (y*y) + (z*z);
  }

  class Function3V
  {
  public:
    Function3V ()
    {
      func3V = f3v;
    }

    double Call (double x, double y, double z)
    {
      return func3V (x, y, z);
    }

  private:
    double (*func3V) (double x, double y, double z);
  };
}

#endif


