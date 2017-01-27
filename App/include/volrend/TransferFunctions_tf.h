#ifndef VOLREND_TRANSFERFUNCTION_tf_H
#define VOLREND_TRANSFERFUNCTION_tf_H

#include <volrend/TransferFunction.h>
#include <lqc/lqcdefines.h>
#include <glm/glm.hpp>

namespace vr
{
  class TransferFunction_tf : public TransferFunction
  {
  private:
    class ControlPoint
    {
    public:
      ControlPoint (double _r, double _g, double _b, double _a, double _iv)
        : r (_r), g (_g), b (_b), a (_a), isovalue (_iv)
      {}

      double r, g, b, a, isovalue;
    };

  public:
    enum Interpolation
    {
      CONSTANT,
      DISCRETE,
      LINEAR,
    };

  public:
    TransferFunction_tf () { m_tflenght = 256; }
    ~TransferFunction_tf () {}

    virtual const char* GetNameClass ()
    {
      return "TrasnferFunction_tf";
    }

    virtual lqc::Vector4d Get (double value)
    {
      for (int i = 0; i < (int)m_ctl_points.size () - 1; i++)
      {
        if (value >= m_ctl_points[i].isovalue && value <= m_ctl_points[i + 1].isovalue)
        {
          lqc::Vector4d v1 = lqc::Vector4d (
            m_ctl_points[i].r,
            m_ctl_points[i].g,
            m_ctl_points[i].b,
            m_ctl_points[i].a);

          lqc::Vector4d v2 = lqc::Vector4d (
            m_ctl_points[i + 1].r,
            m_ctl_points[i + 1].g,
            m_ctl_points[i + 1].b,
            m_ctl_points[i + 1].a);

          double t = (value - m_ctl_points[i].isovalue)
            / (m_ctl_points[i + 1].isovalue - m_ctl_points[i].isovalue);

          return (1.0 - t)*v1 + (t)*v2;
        }
      }
      printf ("Out of Range: [Value = %lf]\n", value);
      return lqc::Vector4d (0.0);
    }

    virtual gl::GLTexture1D* GenerateTexture_RGBA ()
    {
      gl::GLTexture1D* ret = NULL;
      if (m_ctl_points.size () > 0)
      {
        ret = new gl::GLTexture1D (m_tflenght);
        ret->GenerateTexture (GL_LINEAR, GL_LINEAR, GL_CLAMP);

        float* data = new float[m_tflenght * 4];
        for (int i = 0; i < m_tflenght; i++)
        {
          lqc::Vector4d c = Get ((double)i);
          data[(i * 4)] = c.x;
          data[(i * 4) + 1] = c.y;
          data[(i * 4) + 2] = c.z;
          data[(i * 4) + 3] = c.w;
        }

        ret->SetData ((void*)data, GL_RGBA, GL_RGBA, GL_FLOAT);
        delete[] data;
      }

      return ret;
    }

    void SetInterpolationMode (Interpolation interplt)
    {
      m_interpolation = interplt;
    }

    void AddControlPoint (double r, double g, double b, double a, double isovalue)
    {
      m_ctl_points.push_back (ControlPoint (r, g, b, a, isovalue));
    }

    void SetValueLimits (double smin, double smax)
    {
      m_min_limit = smin;
      m_max_limit = smax;
    }

    void CalculateLimits ()
    {
      double isomax, isomin;
      for (int i = 0; i < (int)m_ctl_points.size (); i++)
      {
        if (i == 0)
        {
          isomin = isomax = m_ctl_points[i].isovalue;
        }
        else
        {
          isomin = MIN (isomin, m_ctl_points[i].isovalue);
          isomax = MAX (isomax, m_ctl_points[i].isovalue);
        }
      }
    }

    void ScaleTo (double scalemin, double scalemax)
    {
      for (int i = 0; i < (int)m_ctl_points.size (); i++)
      {
        m_ctl_points[i].isovalue *= scalemax;
      }
    }

  protected:
    std::vector<ControlPoint> m_ctl_points;
    Interpolation m_interpolation;
    int m_tflenght;
    double m_max_limit, m_min_limit;
  private:
  };
}

#endif