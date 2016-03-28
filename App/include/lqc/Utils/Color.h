#ifndef LQC_UTILS_COLOR_H
#define LQC_UTILS_COLOR_H

namespace lqc
{
  typedef struct Color4f
  {
    Color4f ()
    : r (0.0f), g (0.0f), b (0.0f), a (0.0f)
    {}

    Color4f (float R, float G, float B, float A)
      : r (R), g (G), b (B), a (A)
    {}

    Color4f (float C)
      : r (C), g (C), b (C), a (C)
    {}

    static Color4f Zero ()
    {
      return Color4f (0.0f);
    }

    static Color4f One ()
    {
      return Color4f (1.0f);
    }

    Color4f operator-(const Color4f& v)
    {
      Color4f ret;
      ret.r = this->r - v.r;
      ret.g = this->g - v.g;
      ret.b = this->b - v.b;
      ret.a = this->a - v.a;

      return ret;
    }

    Color4f operator+(const Color4f& v)
    {
      Color4f ret;
      ret.r = this->r + v.r;
      ret.g = this->g + v.g;
      ret.b = this->b + v.b;
      ret.a = this->a + v.a;

      return ret;
    }

    Color4f operator*(const float& f)
    {
      Color4f ret;
      ret.r = this->r * f;
      ret.g = this->g * f;
      ret.b = this->b * f;
      ret.a = this->a * f;

      return ret;
    }

    Color4f operator*(const Color4f& f)
    {
      Color4f ret;
      ret.r = this->r * f.r;
      ret.g = this->g * f.g;
      ret.b = this->b * f.b;
      ret.a = this->a * f.a;

      return ret;
    }

    Color4f operator/(const float& f)
    {
      Color4f ret;
      ret.r = this->r / f;
      ret.g = this->g / f;
      ret.b = this->b / f;
      ret.a = this->a / f;

      return ret;
    }

    Color4f operator/(const Color4f& f)
    {
      Color4f ret;
      ret.r = this->r / f.r;
      ret.g = this->g / f.g;
      ret.b = this->b / f.b;
      ret.a = this->a / f.a;

      return ret;
    }

    Color4f& operator=(const Color4f& v)
    {
      this->r = v.r;
      this->g = v.g;
      this->b = v.b;
      this->a = v.a;

      return *this;
    }


    friend Color4f operator*(float a, Color4f b)
    {
      return Color4f (a*b.r, a*b.g, a*b.b, a*b.a);
    }

    float r, g, b, a;
  } Color4f;

  void RGBtoHSV (float r, float g, float b, float *h, float *s, float *v);
  void HSVtoRGB (float *r, float *g, float *b, float h, float s, float v);

  void RGB2HSV (float r, float g, float b,
    float &h, float &s, float &v);

}

#endif