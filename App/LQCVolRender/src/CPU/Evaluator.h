/*!
\file Evaluator.h
\brief Volume Evaluation.
\author Leonardo Quatrin Campagnolo
*/

#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <lqc/lqcdefines.h>

#include "defines.h"

#include <math/MGeometry.h>
#include <atfg/Volume.h>
#include <atfg/TransferFunction.h>
#include <math/Vector3.h>
#include <math/Vector4.h>
#include "SimpsonEvaluation.h"

#include <cassert>
#include <iomanip>      // std::setprecision
#include <fstream>

class VolumeEvaluator;
class AdaptiveSimpsonIntegrator;

class Evaluator
{
public:
  Evaluator ();
  Evaluator (VolumeEvaluator* veva, vr::Volume* vol, vr::TransferFunction* tf);
  ~Evaluator ();

  void SetMinMaxPositions (glm::dvec3 min, glm::dvec3 max);
  void ScaleNormalizedStep (double distmax);
  void SetMaxDepth (int md);

  static double DistanceFunc (double a, double b);
  static double AlphaErrorEvalFunc (double a, double b, double tol_15, double* everror);
  static double ColorErrorEvalFunc (glm::dvec4 a, glm::dvec4 b, double tol_15, glm::dvec3* everror);

  glm::dvec4 color;

protected:
  template <class T>
  class ErrorAlongRay
  {
  public:
    ErrorAlongRay (double ia, double ib, double eclr, T err)
      : a (ia), b (ib), cerror (eclr), erro (err)
    {}

    double a, b;
    double cerror;
    T erro;
  };

  class ColorAlongRay
  {
  public:
    ColorAlongRay (double ia, double ib, glm::dvec4 icolor)
      : s (ia), h (ib), color (icolor)
    {}

    double s, h;
    glm::dvec4 color;
  };


  double GetOpacity (glm::dvec4 color);
  glm::dvec4 GetFromTransferFunction (glm::dvec3 p);
  
  VolumeEvaluator* m_volumeevaluator;
  vr::Volume* volume;
  vr::TransferFunction* transferFunction;

  glm::dvec3 m_minpos;
  glm::dvec3 m_maxpos;
  glm::dvec3 m_normalized_step;

  int maxdepth;

private:

};

class ASEvaluator : public Evaluator
{
private:
  bool equalintervals;

public:
  ASEvaluator () {}
  ASEvaluator (VolumeEvaluator* veva, vr::Volume* vol, vr::TransferFunction* tf);
  ~ASEvaluator ();

  static double f_InnerIntegral (double p, double h, double epsilon, void* data)
  {
    ASEvaluator* d = (ASEvaluator*)data;
    return d->GetOpacity (d->GetFromTransferFunction (d->m_minpos + p * d->m_normalized_step));
  }

  static glm::dvec4 f_OuterIntegral (double p, double h, double epsilon, void* data)
  {
    ASEvaluator* d = (ASEvaluator*)data;
    return d->OuterIntegral (p, h, epsilon, data);
  }
  
  glm::dvec4 OuterIntegral (double p_d, double h, double epsilon, void* data);

  static void whenreturn (glm::dvec4 clr,
                          double a,
                          double b,
                          double errorevaluated,
                          double errorthreshold,
                          int reclevel,
                          double h,
                          glm::dvec3 err,
                          void* data);

  static void InnerReturnFunc (double alpha,
                               double a,
                               double b,
                               double errorevaluated,
                               double errorthreshold,
                               int reclevel,
                               double h,
                               double err,
                               void* data);

  static void beforerecursion (void* data, double error);
  
  double minpos;

  double pre_computed_opacity;
  double pre_computed_alpha_error;

  double rec_sum_alpha_error;
  double rec_current_opacity_eval;
  double rec_minpos;

  double inner_minor_step_subdivision;
  double minor_step_subdivision;

  double inner_estimated_error;
  double inner_evaluated_error;

  void ResetNormalizedStep (glm::dvec3 min, glm::dvec3 max, double dmax, bool normalized = false)
  {
    if (!normalized)
      m_normalized_step = glm::normalize (m_maxpos - m_minpos);
    else
      m_normalized_step = dmax * glm::normalize (m_maxpos - m_minpos);
  }

  void SetAdaptiveInnerIntervalValues (double innerror, double distinterval, int maxd)
  {
    ierror = innerror;
    adaptiveintervals = distinterval;
    maxdepth = maxd;
  }

  void SetInnerOptimization (int innercounter, bool stay1_innercounter)
  {
    inner_counter = innercounter;
    stay_1_inner_counter = stay1_innercounter;
  }

  std::vector<ErrorAlongRay<glm::dvec3>> m_error_ray;
  std::vector<ErrorAlongRay<double>> m_inner_error_ray;
  std::vector<ColorAlongRay> m_color_ray;
protected:
  std::vector<ErrorAlongRay<double>> m_inner_error_ray_recursion;
private:

  double ierror;
  double adaptiveintervals;
  
  //Contador interno de iterações para avaliar a integral interna
  int inner_counter;
  //Utilizado na primeira chamada do simpson adaptativo
  // para deixar o contador setado como 1 (a-c e c-b)
  bool stay_1_inner_counter;

private:
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AdaptiveSimpsonParticionedEvaluator
{
public:
  class Parameters
  {
  public:
    Parameters ()
    {
      interval_length = internal_integral_error = external_integral_error = -1.0;
      simpson_max_depth = -1;
      volume_evaluator = NULL;
      transfer_function = NULL;
      volume = NULL;

      alpha_truncation = false;
      alpha_max_breaker = 0.95;
    }

    Parameters (double i_external_integral_error, double i_internal_integral_error, 
      double i_interval_length, int i_simpson_max_depth,
      vr::Volume* i_volume, vr::TransferFunction* i_transfer_function,
      VolumeEvaluator* i_volume_evaluator)
    {
      external_integral_error = i_external_integral_error;
      internal_integral_error = i_internal_integral_error;

      interval_length         = i_interval_length;

      simpson_max_depth       = i_simpson_max_depth;
      
      volume                  = i_volume;
      transfer_function       = i_transfer_function;
      volume_evaluator        = i_volume_evaluator;
    }

    double external_integral_error;
    double internal_integral_error;

    double interval_length;

    int simpson_max_depth;

    bool alpha_truncation;
    double alpha_max_breaker;

    vr::Volume* volume;
    vr::TransferFunction* transfer_function;
    VolumeEvaluator* volume_evaluator;
  };

  class Result
  {
  public:
    glm::dvec4 color;

    double internal_minstep;
    double external_minstep;

    double estimated_internal_error;
    double evaluated_internal_error;

    double estimated_external_error;
    double evaluated_external_error;
  };

public:
  AdaptiveSimpsonParticionedEvaluator () {}
  ~AdaptiveSimpsonParticionedEvaluator () {}

  void SetParameters (AdaptiveSimpsonParticionedEvaluator::Parameters parameters)
  {
    m_parameters.external_integral_error = parameters.external_integral_error;
    m_parameters.internal_integral_error = parameters.internal_integral_error;
    m_parameters.interval_length         = parameters.interval_length;
    m_parameters.simpson_max_depth       = parameters.simpson_max_depth;
    m_parameters.transfer_function       = parameters.transfer_function;
    m_parameters.volume                  = parameters.volume;
    m_parameters.volume_evaluator        = parameters.volume_evaluator;
  
    m_parameters.alpha_truncation        = parameters.alpha_truncation;
    m_parameters.alpha_max_breaker       = parameters.alpha_max_breaker;
  }

  void SetErrorAndInterval (double external_error, double internal_error, double interval_length)
  {
    m_parameters.external_integral_error = external_error;
    m_parameters.internal_integral_error = internal_error;
    m_parameters.interval_length = interval_length;
  }

  void SetPointers (vr::Volume* i_volume,
    vr::TransferFunction* i_transfer_function,
    VolumeEvaluator* i_volume_evaluator)
  {
    m_parameters.transfer_function = i_transfer_function;
    m_parameters.volume = i_volume;
    m_parameters.volume_evaluator = i_volume_evaluator;
  }

  void Init (glm::dvec3 minpos, glm::dvec3 maxpos)
  {
    m_result.color = glm::dvec4(0, 0, 0, 0);

    data = ASEvaluator (m_parameters.volume_evaluator,
      m_parameters.volume, m_parameters.transfer_function);
    data.pre_computed_alpha_error = 0.0;
    data.SetAdaptiveInnerIntervalValues (m_parameters.internal_integral_error,
      m_parameters.interval_length, m_parameters.simpson_max_depth);
    data.SetMinMaxPositions (minpos, maxpos);

    se.SetFunctions (ASEvaluator::DistanceFunc, ASEvaluator::ColorErrorEvalFunc, ASEvaluator::beforerecursion,
      ASEvaluator::whenreturn, NULL, NULL);
  }

  void RunEvaluation (double s0, double s1)
  {

    double h = m_parameters.interval_length;
    double s = s0;

    while (s < s1)
    {
      h = MIN (h, s1 - s);

      data.SetInnerOptimization (0, true);
      m_result.color += se.AdaptiveSimpsons (ASEvaluator::f_OuterIntegral, s, s + h,
        (m_parameters.external_integral_error * h) / m_parameters.interval_length,
        m_parameters.simpson_max_depth, &data);
      
#ifdef ANALYSIS__MINMAX_STEPS
      m_result.estimated_internal_error +=
        (m_parameters.external_integral_error * h)
        / m_parameters.interval_length;
      if (h != s1 - s)
      {
        m_result.internal_minstep = data.inner_minor_step_subdivision;
        m_result.external_minstep = data.minor_step_subdivision;
      }
#endif

      s = s + h;
    }

    if (m_parameters.alpha_truncation)
      m_result.color.w = m_parameters.alpha_max_breaker;

#ifdef ANALYSIS__MINMAX_STEPS
    m_result.estimated_external_error += se.estimated_cumulative_error;

    m_result.evaluated_internal_error += data.pre_computed_alpha_error;
    m_result.evaluated_external_error += se.evaluated_cumulative_error;
#endif

  }

  void End (AdaptiveSimpsonParticionedEvaluator::Result* results)
  {
    (*results).color = m_result.color;
  
#ifdef ANALYSIS__ERROR_ALONG_THE_RAY
    SaveErrorAlongTheRay ();
#endif

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
    SaveRGBAAlongTheRay ();
#endif
  }

  ASEvaluator* GetDataEvaluator ()
  {
    return &data;
  }

protected:
private:
  void SaveErrorAlongTheRay ()
  {
    std::ofstream color_error;
    std::ofstream alpha_error;
    color_error.open ("ASP color_error.txt");
    alpha_error.open ("ASP alpha_error.txt");
    for (int i = 0; i < data.m_error_ray.size (); i++)
    {
      color_error << std::setprecision (30) << data.m_error_ray[i].a << '\t'
        << std::setprecision (30) << data.m_error_ray[i].b - data.m_error_ray[i].a << '\t'
        << -1 << '\t'
        << std::setprecision (30) << data.m_error_ray[i].cerror << '\t'
        << std::setprecision (30) << data.m_error_ray[i].erro.x << '\t'
        << std::setprecision (30) << data.m_error_ray[i].erro.y << '\t'
        << std::setprecision (30) << data.m_error_ray[i].erro.z << '\t'
        << std::setprecision (30) << -1 << '\n';
    }
    for (int i = 0; i < data.m_inner_error_ray.size (); i++)
    {
      alpha_error << std::setprecision (30) << data.m_inner_error_ray[i].a << '\t'
        << std::setprecision (30) << data.m_inner_error_ray[i].b - data.m_inner_error_ray[i].a << '\t'
        << std::setprecision (30) << data.m_inner_error_ray[i].cerror << '\t'
        << std::setprecision (30) << data.m_inner_error_ray[i].erro << '\n';
    }
    color_error.close ();
    alpha_error.close ();
  }

  void SaveRGBAAlongTheRay ()
  {
    std::ofstream rgba_ray_file;
    rgba_ray_file.open ("ASP RGBA.txt");
    for (int i = 0; i < data.m_color_ray.size (); i++)
      rgba_ray_file << data.m_color_ray[i].s << '\t'
      << data.m_color_ray[i].h << '\t'
      << data.m_color_ray[i].color.x << '\t'
      << data.m_color_ray[i].color.y << '\t'
      << data.m_color_ray[i].color.z << '\t'
      << data.m_color_ray[i].color.w << '\n';
    rgba_ray_file.close ();
  }

  AdaptiveSimpsonParticionedEvaluator::Parameters m_parameters;
  AdaptiveSimpsonParticionedEvaluator::Result m_result;

  ASEvaluator data;
  AdaptiveSimpsonEvaluator<glm::dvec4, double, glm::dvec3> se;
};

#endif