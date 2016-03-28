#include "Viewer.h"

int main (int argc, char **argv)
{
  /*
  double v[10 * 4] = {
    0.5, 0.3, 0.2, 0.5,
      0.5, 0.3, 0.2, 0.5,
      0.4, 0.3, 0.2, 0.2,
      0.5, 0.2, 0.2, 0.05,
      0.2, 0.3, 0.2, 0.6,
      0.5, 0.3, 0.2, 0.1,
      0.5, 0.5, 0.02, 0.5,
      0.05, 0.3, 0.2, 0.3,
      0.5, 0.3, 0.2, 0.03,
      0.5, 0.9, 0.2, 0.5
  };

  double a = 0;
  double dst[3] = { 0, 0, 0 };
  double src[4];
  double colorAcum[4] = { 0, 0, 0, 0 };
  double transparencyAcum = 1.0;
  double h = 1;
  for (int i = 0; i < 10; i++)
  {
    src[0] = v[i*4 + 0];
    src[1] = v[i*4 + 1];
    src[2] = v[i*4 + 2];
    src[3] = v[i*4 + 3];

    double d = h;
    colorAcum[0] += src[0] * src[3]*d * transparencyAcum;
    colorAcum[1] += src[1] * src[3]*d * transparencyAcum;
    colorAcum[2] += src[2] * src[3]*d * transparencyAcum;
    colorAcum[3] += src[3] * d       * transparencyAcum;
    transparencyAcum *= (1.0 - src[3]*d);

    dst[0] += src[0] * src[3] * (1 - a);
    dst[1] += src[1] * src[3] * (1 - a);
    dst[2] += src[2] * src[3] * (1 - a);
    a += src[3] * (1.0 - a);
  }
  printf ("1:\n %lf\n %lf \n%lf \n%lf \n", colorAcum[0], colorAcum[1], colorAcum[2], colorAcum[3]);
  printf ("2:\n %lf\n %lf \n%lf \n%lf \n", dst[0], dst[1], dst[2], a);
  getchar ();
  */


  Viewer::Instance ()->InitAndStart (argc, argv);
  Viewer::DestroyInstance ();
  return 0;
}