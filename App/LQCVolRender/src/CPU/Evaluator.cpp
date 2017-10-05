#include "Evaluator.h"

#include "VolumeEvaluator.h"

#include <math/MGeometry.h>
#include <math/Vector3.h>
#include <math/Vector4.h>

#include <lqc/lqcdefines.h>
#include <lqc/Utils/Utils.h>

// Evaluator

/////////////////////
// public methods  //
/////////////////////

Evaluator::Evaluator ()
: m_volumeevaluator (NULL), volume (NULL), transferFunction (NULL) 
{
  color = glm::dvec4 (0);
}

Evaluator::Evaluator (VolumeEvaluator* veva, vr::Volume* vol, vr::TransferFunction* tf)
: m_volumeevaluator (veva), volume (vol), transferFunction (tf)
{
  color = glm::dvec4 (0);
}

Evaluator::~Evaluator () {}

void Evaluator::SetMinMaxPositions (glm::dvec3 min, glm::dvec3 max)
{
  m_minpos = min;
  m_maxpos = max;
  m_normalized_step = glm::normalize (m_maxpos - m_minpos);
}

void Evaluator::ScaleNormalizedStep (double distmax)
{
  m_normalized_step = distmax * m_normalized_step;
}

void Evaluator::SetMaxDepth (int md)
{
  maxdepth = md;
}

double Evaluator::DistanceFunc (double a, double b)
{
  return fabs (b - a);
}

double Evaluator::AlphaErrorEvalFunc (double a, double b, double tol_15, double* everror)
{
  (*everror) = fabs (b - a);

  return fabs (b - a);
}

double Evaluator::ColorErrorEvalFunc (glm::dvec4 a, glm::dvec4 b, double tol_15, glm::dvec3* everror)
{
  (*everror) = glm::dvec3 (fabs (a.x - b.x), fabs (a.y - b.y), fabs (a.z - b.z));

	double manhattandist = abs(a.x - b.x) + abs(a.y - b.y) + abs(a.z - b.z);
  double colordist = glm::distance (glm::dvec3 (a.x, a.y, a.z), glm::dvec3 (b.x, b.y, b.z));
  if (manhattandist < (1.0 / pow (10, 6)))
    return 0.0;
  return colordist;
}

/////////////////////
//protected methods//
/////////////////////

double Evaluator::GetOpacity (glm::dvec4 color)
{
  return color.w;
}

glm::dvec4 Evaluator::GetFromTransferFunction (glm::dvec3 p)
{
  glm::dvec4 ret;
  if (!transferFunction || !volume) ret = glm::dvec4 (0.0);
  else ret = transferFunction->Get (m_volumeevaluator->GetValueFromVolume (volume, glm::vec3 (p.x, p.y, p.z)));
  return ret;
}

/////////////////////
// private methods //
/////////////////////

// Evaluator*
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ASEvaluator

/////////////////////
// public methods  //
/////////////////////

ASEvaluator::ASEvaluator (VolumeEvaluator* veva, vr::Volume* vol, vr::TransferFunction* tf)
: Evaluator(veva, vol, tf),
rec_current_opacity_eval (0.0),
pre_computed_opacity (0.0),
inner_counter (0),
stay_1_inner_counter (true),
minor_step_subdivision (999999999),
inner_minor_step_subdivision (999999999),
minpos (0),
rec_minpos (0)
{
  inner_estimated_error = 0;
  inner_evaluated_error = 0;

  equalintervals = false;
}

ASEvaluator::~ASEvaluator ()
{
}

glm::dvec4 ASEvaluator::OuterIntegral (double p_d, double h, double epsilon, void* data)
{
  AdaptiveSimpsonEvaluator<double, double, double> se;
  se.SetFunctions (ASEvaluator::DistanceFunc, ASEvaluator::AlphaErrorEvalFunc,
                   NULL, ASEvaluator::InnerReturnFunc, NULL, NULL);
  glm::dvec3 p = m_minpos + p_d * m_normalized_step;
#ifdef EVALUATIONINTEGRAL_CALCULATE_MINOR_STEP
  se.SetReturnFunction (ASEvaluator::InnerReturnFunc);
#endif

  glm::dvec4 C = GetFromTransferFunction (p);
  double tau = GetOpacity (C);

  double ois = 0.0;
  double err = ierror;
  double innerint = 0.0;

  if (err > 0.0)
  {
    if (equalintervals)
    {
      double pi = rec_minpos;
      double pf = pi + h;
      for (int i = 0; i < inner_counter; i++)
      {
        double scaled_error = (ierror * (pf - pi)) / adaptiveintervals;
        ois += se.AdaptiveSimpsons (ASEvaluator::f_InnerIntegral,
                                    pi, pf, scaled_error, maxdepth, this);
        pi = pf;
        pf = pi + h;
      }
      rec_current_opacity_eval += ois;
    }
    else
    {
      double pi = rec_minpos;
      double pf = p_d;
      double scaled_error = (ierror * (pf - pi)) / adaptiveintervals;
      ois += se.AdaptiveSimpsons (ASEvaluator::f_InnerIntegral,
                                  pi, pf, scaled_error, maxdepth, this);
      rec_current_opacity_eval += ois;
    }
  }

  if (stay_1_inner_counter)
    inner_counter = 1;
  else
    inner_counter++;

  rec_minpos = p_d;

  innerint =
    //1 - (rec_current_opacity_eval + pre_computed_opacity)
    exp (-(rec_current_opacity_eval + pre_computed_opacity))
    ;

  double alphachannel = tau*innerint;
  return glm::dvec4 (
    alphachannel * C.x,
    alphachannel * C.y,
    alphachannel * C.z,
    alphachannel
    );
}

void ASEvaluator::whenreturn (glm::dvec4 clr,
                              double a,
                              double b,
                              double errorevaluated,
                              double errorthreshold,
                              int reclevel,
                              double h,
                              glm::dvec3 err,
                              void* data)
{
  ASEvaluator* d = (ASEvaluator*)data;
  AdaptiveSimpsonEvaluator<double, double, double> se;
  se.SetFunctions (ASEvaluator::DistanceFunc, ASEvaluator::AlphaErrorEvalFunc,
                   NULL, ASEvaluator::InnerReturnFunc, NULL, NULL);
  if (d->equalintervals)
  {
    if (d->ierror > 0.0)
    {
      double scaled_error = (d->ierror * (b - d->rec_minpos)) / d->adaptiveintervals;
      double ois = se.AdaptiveSimpsons (ASEvaluator::f_InnerIntegral,
                                        d->rec_minpos, b, scaled_error, d->maxdepth, data);
      d->rec_minpos = b;
      d->rec_current_opacity_eval += ois;
    }
    
    d->minpos = b;
    d->rec_minpos = b;
  }
  else
  {
    d->minpos = d->rec_minpos;
  }

  d->pre_computed_opacity += d->rec_current_opacity_eval;
  d->pre_computed_alpha_error += d->rec_sum_alpha_error;

#ifdef EVALUATIONINTEGRAL_CALCULATE_MINOR_STEP
  if (d->minor_step_subdivision < 0) d->minor_step_subdivision = glm::distance (a, b);
  else d->minor_step_subdivision = MIN (d->minor_step_subdivision, glm::distance (a, b));
#endif

#ifdef ANALYSIS__ERROR_ALONG_THE_RAY
  d->m_error_ray.push_back (ErrorAlongRay<glm::dvec3>(a, b, errorevaluated/15.0, err));

  d->m_inner_error_ray.insert (d->m_inner_error_ray.end (),
                               d->m_inner_error_ray_recursion.begin (), 
                               d->m_inner_error_ray_recursion.end ());
#endif

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
  d->color += clr;
  ColorAlongRay exce(b, b-a, d->color);
  d->m_color_ray.push_back (exce);
#endif

}

void ASEvaluator::InnerReturnFunc (double alpha,
                                   double a,
                                   double b,
                                   double errorevaluated,
                                   double errorthreshold,
                                   int reclevel,
                                   double h,
                                   double err,
                                   void* data)
{
  ASEvaluator* d = (ASEvaluator*)data;
#ifdef EVALUATIONINTEGRAL_CALCULATE_MINOR_STEP
  if (d->inner_minor_step_subdivision < 0) d->inner_minor_step_subdivision = glm::distance (a, b);
  else d->inner_minor_step_subdivision = MIN (d->inner_minor_step_subdivision, glm::distance (a, b));
#endif

#ifdef ANALYSIS__ERROR_ALONG_THE_RAY
  d->m_inner_error_ray_recursion.push_back (ErrorAlongRay<double> (a, b, errorevaluated / 15.0, err));
#endif
  d->rec_sum_alpha_error += errorevaluated;
}

void ASEvaluator::beforerecursion (void* data, double error)
{
  ASEvaluator* d = (ASEvaluator*)data;
  d->rec_sum_alpha_error = 0.0;
  d->rec_current_opacity_eval = 0.0;
  d->rec_minpos = d->minpos;
  d->inner_counter = 1;
  d->stay_1_inner_counter = false;

#ifdef ANALYSIS__ERROR_ALONG_THE_RAY
  d->m_inner_error_ray_recursion.clear ();
#endif
}



/////////////////////
//protected methods//
/////////////////////

/////////////////////
// private methods //
/////////////////////

// ASEvaluator*