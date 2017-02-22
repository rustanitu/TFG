/*!
\file   SimpsonErrorHalfIntegrator.h
\brief  Simpson Integrator based on error value
\author Leonardo Quatrin Campagnolo
*/

#ifndef SIMPSON_ERROR_HALF_INTEGRATOR_H
#define SIMPSON_ERROR_HALF_INTEGRATOR_H

#include <cstdlib>
#include <math/Vector3.h>
#include <math/Vector4.h>
#include <math/MGeometry.h>

#include <volrend/Volume.h>
#include <volrend/TransferFunction.h>
#include "defines.h"
#include <algorithm>

class VolumeEvaluator;

class SimpsonErrorHalfIntegrator
{
public:
  static int s_IntegrativeExternalTimes;
  static int s_IntegrativeInternalTimes;

  static int s_RecalculateIntegrativeExternalTimes;
  static int s_RecalculateIntegrativeInternalTimes;

  bool ColorErrorEvalFunc (glm::dvec4 a, glm::dvec4 b, double tol);
  bool AlphaErrorEvalFunc (double a, double b, double tol);

public:
  class ExternalCumulativeError
  {
  public:
    double s;
    double h;
    glm::dvec4 err;
  };
  std::vector<ExternalCumulativeError> m_excumulative_error;

#ifdef ANALYSIS__ERROR_ALONG_THE_RAY

  class Analysis_ErrorAlongTheRay
  {
  public:
    class Ext_IntervalError
    {
    public:
      Ext_IntervalError (double s2, double h2, glm::dvec4 err2)
        : err (err2), s (s2), h (h2) {}
      Ext_IntervalError () : err (0), s (0), h (0) {}
      double s;
      double h;
      glm::dvec4 err;
    };

    class Int_IntervalError
    {
    public:
      Int_IntervalError (double s2, double h2, double err2)
        : err (err2), s (s2), h (h2) {}
      Int_IntervalError () : err (0), s (0), h (0) {}
      double s;
      double h;
      double err;
    };
  public:
    Analysis_ErrorAlongTheRay () {}

    void PrintResults (std::string internal_file, std::string external_file)
    {
      std::ofstream internal_error_file;
      internal_error_file.open (internal_file);
      double interr = 0;
      for (int i = 0; i < m_int_errorintervals.size (); i++)
      {
        interr += m_int_errorintervals[i].err;
        internal_error_file << m_int_errorintervals[i].s + m_int_errorintervals[i].h << '\t'
          << std::setprecision (30) << interr << '\n';
      }
      internal_error_file.close ();
   
      std::ofstream external_error_file;
      external_error_file.open (external_file);
      glm::dvec4 exterr (0);
      for (int i = 0; i < m_ext_errorintervals.size (); i++)
      {
        exterr += m_ext_errorintervals[i].err;

        external_error_file << m_ext_errorintervals[i].s + m_ext_errorintervals[i].h << '\t'
          << std::setprecision (30) << exterr.x << '\t'
          << std::setprecision (30) << exterr.y << '\t'
          << std::setprecision (30) << exterr.z << '\t'
          << std::setprecision (30) << exterr.w << '\n';
      }
      external_error_file.close ();

    }

    std::vector<Ext_IntervalError> m_ext_errorintervals;
    std::vector<Int_IntervalError> m_int_errorintervals;
  private:
  };
#endif

public:
  SimpsonErrorHalfIntegrator (VolumeEvaluator* veva);
  ~SimpsonErrorHalfIntegrator ();

  void Init (glm::dvec3 minp, glm::dvec3 maxp, vr::Volume* vol, vr::TransferFunction* tf);
  void Integrate (double s0, double s1, double tol, double h0);

  glm::dvec4 color;
  double pre_integrated;
  double minpost;
  glm::dvec4 Cminpost;
protected:
  bool IntegrateInternalInterval (double a, double b, double tol, double* pS, double* pSleft, double* pSright, glm::dvec4 clr[], bool force);
  double AdaptiveInternalIntegration (double s, double h0, double tol, double values[], glm::dvec4 clr[]);

  glm::dvec4 ExternalIntegration (glm::dvec4 C, double p_d, double inner);
  bool TryExternaIntegration (double s, double h, double tol, double v[], glm::dvec4 clr[]);

  void AdaptiveExternalIntegration (double s, double h, double tol, double* pre_integrated);
  glm::dvec4 ExtenalEvaluation (double p_d, glm::dvec4 C);
  bool IntegrateExternalInterval (double a, double b, double tol, glm::dvec4* pS, glm::dvec4 clr[], bool force);
  double Aux_AdaptiveExternalIntegration (double s, double h, double tol, double* pre_integrated);

  glm::dvec4 GetFromTransferFunction (double p_d);

private:
  double MaxExternalError (glm::dvec4 err)
  {
    return std::max (std::max (err.x, err.y), std::max (err.z, err.w));
  }

  VolumeEvaluator* volume_evaluator;
  vr::Volume* volume;
  vr::TransferFunction* transfer_function;

  glm::dvec3 minpos;
  glm::dvec3 maxpos;
  glm::dvec3 normalized_step;

#ifdef ANALYSIS__ERROR_ALONG_THE_RAY
  Analysis_ErrorAlongTheRay error_along_the_ray;
  glm::dvec4 ext_aux_error;
  glm::dvec4 ext_left_error;
  glm::dvec4 ext_right_error;
  double int_aux_error;
  double int_left_error;
  double int_right_error;
#endif

  double internal_sum_error;
  double internal_error_aux;

  glm::dvec4 external_sum_error;
  glm::dvec4 external_error_aux;

  glm::dvec4 last_color;
};

#endif