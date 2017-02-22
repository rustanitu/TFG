/*!
\file   SimpsonErrorQuadraticIntegrator.h
\brief  Simpson Integrator based on error value and quadratic integration
\author Leonardo Quatrin Campagnolo
*/

#ifndef SIMPSON_ERROR_QUADRATIC_INTEGRATOR_H
#define SIMPSON_ERROR_QUADRATIC_INTEGRATOR_H

#include <cstdlib>
#include <math/Vector3.h>
#include <math/Vector4.h>
#include <math/MGeometry.h>

#include <volrend/Volume.h>
#include <volrend/TransferFunction.h>
#include "defines.h"

#include <algorithm>

class VolumeEvaluator;

class SimpsonErrorQuadraticIntegrator
{
public:
  static int s_IntegrativeExternalTimes;
  static int s_IntegrativeInternalTimes;

  static int s_RecalculateIntegrativeExternalTimes;
  static int s_RecalculateIntegrativeInternalTimes;

public:
  SimpsonErrorQuadraticIntegrator (VolumeEvaluator* veva);
  ~SimpsonErrorQuadraticIntegrator ();

  void Reset ();

  void Init (glm::dvec3 minp, glm::dvec3 maxp, vr::Volume* vol, vr::TransferFunction* tf);
  void Integrate (double s0, double s1, double tol, double h0);
  
  glm::dvec4 color;

  double m_clc_proj_internal_error;
  glm::dvec4 m_clc_proj_external_error;

protected:
  int m_integrate_method;
  void IntegrateError (double s0, double s1, double tol, double h0);
  void IntegrateProjection (double s0, double s1, double tol, double h0);
  
  bool ColorErrorEvalFunc (glm::dvec4 a, glm::dvec4 b, double tol);
  double IntegrateInternalInterval (double s, double h, double tol);

  glm::dvec4 ExtenalEvaluation (double p_d, glm::dvec4 C, double* pre_alpha = NULL);
  glm::dvec4 ExtenalEvaluationMiddle (double p_d, glm::dvec4 C, glm::dvec4 Cmid, double* pre_alpha = NULL);
  bool IntegrateExternalInterval (double s, double h, double tol, double* pre_alpha);

  glm::dvec4 GetFromTransferFunction (double p_d);

  void CalculateInternalError (double s, double h, double tol);
  void CalculateExternalError (double s, double h, double tol);

private:
  double MaxExternalError (glm::dvec4 err)
  {
    return std::max (std::max (err.x, err.y), std::max (err.z, err.w));
  }

  double pre_integrated;
  double minpost;
  glm::dvec4 Cminpost;

  VolumeEvaluator* volume_evaluator;
  vr::Volume* volume;
  vr::TransferFunction* transfer_function;

  glm::dvec3 minpos;
  glm::dvec3 maxpos;
  glm::dvec3 normalized_step;

  glm::dvec4 external_error_aux;
};

#endif