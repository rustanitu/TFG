#include <lqc/Utils/LHHistogram.h>

#include <lqc/lqcdefines.h>
#include <math/MUtils.h>
#include <math/Vector4.h>
#include <math/Vector3.h>

#include <cstdlib>
#include <iostream>
#include <fstream>

namespace lqc
{
  LHHistogram::LHHistogram ()
  {
    m_width = 255;
    m_height = 255;
    //Clear();
  }

  LHHistogram::LHHistogram (int width, int height)
    : m_width(width), m_height(height)
  {
    //Clear();
  }

  LHHistogram::~LHHistogram ()
  {
    //Clear();
  }
}