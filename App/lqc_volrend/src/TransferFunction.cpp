#include "TransferFunction.h"

namespace vr
{
  TransferFunction::TransferFunction()
  : m_sigma(0.0f)
  , m_gaussian_bx(true)
  , m_thickness(1)
  , m_distmap(NULL)
  {

  }
  
  TransferFunction::~TransferFunction()
  {
    if (m_distmap)
      m_distmap->CleanUp();
    delete m_distmap;
  }

  double TransferFunction::CenteredTriangleFunction(double x, double max, double base, double center)
  {
    //  boundary center
    //         .
    //        / \       |
    //       / | \      |
    //      /  |  \     |
    //     /   |   \    | max
    //    /    |    \   |
    //   /     |     \  |
    //  /      |      \ |
    //  ---- center ++++ 
    // |-------|-------|
    //       base

    double a = 0.0f;
    if (x >= -base && x <= base)
    {
      if (x >= center && center < base)
      {
        a = -(max * x) / (base - center);
        a += (max * base) / (base - center);
      }
      else
      {
        a = (max * x) / (base + center);
        a += (max * base) / (base + center);
      }
    }

    return fmin(a, 1.0f);
  }

  double TransferFunction::CenteredGaussianFunction(double x, double max, double base, double u)
  {
    double sigma = base / 3.0f;
    double gauss = max * exp(((-(x - u)*(x - u)) / (2 * sigma * sigma)));
    return fmin(gauss, 1.0f);
  }
}