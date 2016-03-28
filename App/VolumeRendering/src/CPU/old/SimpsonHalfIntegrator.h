/*!
\file SimpsonHalfIntegrator.h
\brief Adaptive SimpsonHalfIntegrator
\author Leonardo Quatrin Campagnolo
*/

#ifndef SIMPSONHALFINTEGRATION_H
#define SIMPSONHALFINTEGRATION_H

#include <cstdlib>
#include <math/Vector3.h>
#include <math/Vector4.h>
#include <math/MGeometry.h>

#include <volrend/Volume.h>
#include <volrend/TransferFunction.h>
#include "defines.h"
#include "VolumeEvaluator.h"

#include <algorithm>

class AdaptiveSimpsonHalfIntegrator
{
public:
  static double s_acumulated_internal_error;
  static double s_acumulated_external_error;
  static int s_IntegrativeExternalTimes;
  static int s_IntegrativeInternalTimes;

  static int s_RecalculateIntegrativeExternalTimes;
  static int s_RecalculateIntegrativeInternalTimes;
public:
  class ExternalCumulativeError
  {
  public:
    double s;
    double h;
    lqc::Vector4d err;
  };
  std::vector<ExternalCumulativeError> m_excumulative_error;


  class InternalErrorBeyondLimit
  {
  public:
    InternalErrorBeyondLimit () : error (0), s (0), h (0) {}
    InternalErrorBeyondLimit (double er, double si, double ht)
      : error (er), s (si), h (ht) {}
    double error;
    double s;
    double h;
  };
  std::vector<InternalErrorBeyondLimit> m_int_beyonderrorlimit;

  class ExternalErrorBeyondLimit
  {
  public:
    ExternalErrorBeyondLimit () : error (0), s (0), h (0) {}
    ExternalErrorBeyondLimit (double er, double si, double ht)
      : error (er), s (si), h (ht) {}
    double error;
    double s;
    double h;
  };
  std::vector<ExternalErrorBeyondLimit> m_ext_beyonderrorlimit;


#ifdef ANALYSIS__ERROR_ALONG_THE_RAY

  class Analysis_ErrorAlongTheRay
  {
  public:
    class Ext_IntervalError
    {
    public:
      Ext_IntervalError (double s2, double h2, lqc::Vector4d err2)
        : err (err2), s (s2), h (h2) {}
      Ext_IntervalError () : err (0), s (0), h (0) {}
      double s;
      double h;
      lqc::Vector4d err;
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
      lqc::Vector4d exterr (0);
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
  AdaptiveSimpsonHalfIntegrator (lqc::Vector3d minp, lqc::Vector3d maxp, VolumeEvaluator* veva,
                             vr::Volume* vol, vr::TransferFunction* tf);
  ~AdaptiveSimpsonHalfIntegrator ();

  void Integrate (double s0, double s1, double tol, double h0);

  bool ColorErrorEvalFunc (lqc::Vector4d a, lqc::Vector4d b, double tol)
  {
    tol = 15.0 * tol;

    double error_r = fabs (a.x - b.x);
    double error_g = fabs (a.y - b.y);
    double error_b = fabs (a.z - b.z);
    double error_a = fabs (a.w - b.w);

#ifdef ANALYSIS__ERROR_ALONG_THE_RAY
    ext_aux_error = lqc::Vector4d (error_r, error_g, error_b, error_a);
#endif

#ifdef INTEGRATOR__STEP_BASED_ON_ERROR
    external_error_aux = std::max (std::max(error_r, error_g),
                                  std::max(error_b, error_a));
#endif
    
    return (tol >= error_r && tol >= error_g 
      && tol >= error_b && tol >= error_a);
  }

  bool AlphaErrorEvalFunc (double a, double b, double tol)
  {
    tol = 15.0 * tol;
    double alphaerror = fabs (b - a);

#ifdef ANALYSIS__ERROR_ALONG_THE_RAY
    int_aux_error = alphaerror;
#endif

#ifdef INTEGRATOR__STEP_BASED_ON_ERROR
    internal_error_aux = alphaerror;
#endif
    return tol >= alphaerror;
  }

  lqc::Vector4d color;
  double pre_integrated;
  //double rec_current_opacity_eval;
  double minpost;
  lqc::Vector4d Cminpost;
protected:
  bool IntegrateInternalInterval (double a, double b, double tol, double* pS, double* pSleft, double* pSright, lqc::Vector4d clr[], bool force, double Sprecalculated);
  double AdaptiveInternalIntegration (double s, double h0, double tol, double values[], lqc::Vector4d clr[]);

  lqc::Vector4d ExternalIntegration (lqc::Vector4d C, double p_d, double inner);
  bool TryExternaIntegration (double s, double h, double tol, double v[], lqc::Vector4d clr[]);

  void AdaptiveExternalIntegration (double s, double h, double tol, double* pre_integrated);
  lqc::Vector4d ExtenalEvaluation (double p_d, lqc::Vector4d C);
  bool IntegrateExternalInterval (double a, double b, double tol, lqc::Vector4d* pS, lqc::Vector4d clr[], bool force, lqc::Vector4d Spre);
  double Aux_AdaptiveExternalIntegration (double s, double h, double tol, double* pre_integrated);

  lqc::Vector4d GetFromTransferFunction (double p_d);

  void Integrate_InitAnalysisAuxVariables ();
  void Integrate_ResetAnalysisAuxVariables ();
  double Integrate_CalculateNextInitialInterval (double s, double h, double max_tol, double h0, double s1);
  
  void InternalIntegration_SetLeftAuxVariables ();
  void InternalIntegration_SetRightAuxVariables ();
  void InternalIntegration_EndIntegration (double s, double h_2);

  void ExternalIntegration_SetLeftAuxVariables ();
  void ExternalIntegration_SetRightAuxVariables ();
  void ExternalIntegration_EndIntegration (double s, double h_2, double h);

private:
  VolumeEvaluator* volume_evaluator;
  vr::Volume* volume;
  vr::TransferFunction* transfer_function;

  lqc::Vector3d minpos;
  lqc::Vector3d maxpos;
  lqc::Vector3d normalized_step;

#ifdef ANALYSIS__ERROR_ALONG_THE_RAY
  Analysis_ErrorAlongTheRay error_along_the_ray;
  lqc::Vector4d ext_aux_error;
  lqc::Vector4d ext_left_error;
  lqc::Vector4d ext_right_error;
  double int_aux_error;
  double int_left_error;
  double int_right_error;
#endif

};

#endif