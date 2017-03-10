/*!
\file   SimpsonErrorProjectionIntegrator.h
\brief  Simpson Integrator based on error value
\author Leonardo Quatrin Campagnolo
*/

#ifndef SIMPSON_ERROR_PROJECTION_INTEGRATOR_H
#define SIMPSON_ERROR_PROJECTION_INTEGRATOR_H

#include <cstdlib>
#include <math/Vector3.h>
#include <math/Vector4.h>
#include <math/MGeometry.h>

#include <atfg/Volume.h>
#include <atfg/TransferFunction.h>
#include "defines.h"

#include <algorithm>

class VolumeEvaluator;

class SimpsonErrorProjectionIntegrator
{
public:
  static int s_IntegrativeExternalTimes;
  static int s_IntegrativeInternalTimes;

  static int s_RecalculateIntegrativeExternalTimes;
  static int s_RecalculateIntegrativeInternalTimes;

public:
  SimpsonErrorProjectionIntegrator (VolumeEvaluator* veva);
  ~SimpsonErrorProjectionIntegrator ();

  void Reset ();

  void Init (glm::dvec3 minp, glm::dvec3 maxp, vr::Volume* vol, vr::TransferFunction* tf);
  void Integrate (double s0, double s1, double tol, double h0);
  
  glm::dvec4 color;

  double m_clc_proj_newinternalprojection;
  double m_clc_proj_internal_error;
  glm::dvec4 m_clc_proj_external_error;

protected:
  int m_integrate_method;
  void IntegrateProjection (double s0, double s1, double tol, double h0);
  
  bool ColorErrorEvalFunc (glm::dvec4 a, glm::dvec4 b, double tol);
  double IntegrateInternalInterval (double s, double h, double tol);

  glm::dvec4 ExtenalEvaluation (double p_d, glm::dvec4 C, double* pre_alpha = NULL);
  glm::dvec4 ExtenalEvaluationMiddle (double p_d, glm::dvec4 C, glm::dvec4 Cmid, double* pre_alpha = NULL);
  bool IntegrateExternalInterval (double s, double h, double tol, double* pre_alpha, bool minorstep);

  glm::dvec4 GetFromTransferFunction (double p_d);

  double CalculateInternalError (double s, double h, double tol, bool higher = false);
  glm::dvec4 CalculateExternalError (double s, double h, double tol);

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