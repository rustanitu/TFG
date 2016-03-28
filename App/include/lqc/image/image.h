#ifndef LQC_IMAGE_H
#define LQC_IMAGE_H

#include <cstdio>
#include <cstdlib>

namespace lqc
{
  template <typename type>
  type* SummedAreaTable (type* t, unsigned int width, unsigned int height)
  {
    type* sat = new type[width*height];

    sat[0] = t[0];

    for (int x = 1; x < width; x++)
      sat[x] = t[x] + sat[x - 1];

    for (int y = 1; y < height; y++)
    {
      sat[y * width] = t[y * width] + sat[(y - 1) * width];
      for (int x = 1; x < width; x++)
        sat[x + (y * width)] = t[x + (y * width)] + sat[(x - 1) + (y * width)];
    }

    return sat;
  }
}

#endif