#include <lqc/Utils/Color.h>
#include <algorithm>

namespace lqc
{
  // r,g,b values are from 0 to 1
  // h = [0,360], s = [0,1], v = [0,1]
  //		if s == 0, then h = -1 (undefined)
  void RGBtoHSV( float r, float g, float b, float *h, float *s, float *v )
  {
    float min, max, delta;
    min = std::min(std::min(r, g), b);
    max = std::max(std::max(r, g), b);
    *v = max;				// v
    delta = max - min;
    if( max != 0 )
      *s = delta / max;		// s
    else {
      // r = g = b = 0		// s = 0, v is undefined
      *s = 0;
      *h = -1;
      return;
    }
    if( r == max )
      *h = ( g - b ) / delta;		// between yellow & magenta
    else if( g == max )
      *h = 2 + ( b - r ) / delta;	// between cyan & yellow
    else
      *h = 4 + ( r - g ) / delta;	// between magenta & cyan
    *h *= 60;				// degrees
    if( *h < 0 )
      *h += 360;
  }

  void HSVtoRGB( float *r, float *g, float *b, float h, float s, float v )
  {
    int i;
    float f, p, q, t;
    if( s == 0 ) {
      // achromatic (grey)
      *r = *g = *b = v;
      return;
    }
    h /= 60;			// sector 0 to 5
    i = (int)floor( h );
    f = h - i;			// factorial part of h
    p = v * ( 1 - s );
    q = v * ( 1 - s * f );
    t = v * ( 1 - s * ( 1 - f ) );
    switch( i ) {
    case 0:
      *r = v;
      *g = t;
      *b = p;
      break;
    case 1:
      *r = q;
      *g = v;
      *b = p;
      break;
    case 2:
      *r = p;
      *g = v;
      *b = t;
      break;
    case 3:
      *r = p;
      *g = q;
      *b = v;
      break;
    case 4:
      *r = t;
      *g = p;
      *b = v;
      break;
    default:		// case 5:
      *r = v;
      *g = p;
      *b = q;
      break;
    }
  }

  void RGB2HSV(float r, float g, float b, float &h, float &s, float &v)
  {
    float K = 0.f;

    if (g < b)
    {
      std::swap(g, b);
      K = -1.f;
    }

    if (r < g)
    {
      std::swap(r, g);
      K = -2.f / 6.f - K;
    }

    float chroma = r - std::min(g, b);
    h = fabs(K + (g - b) / (6.f * chroma + 1e-20f));
    s = chroma / (r + 1e-20f);
    v = r;
  }


}