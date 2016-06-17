/*!
\file SimpsonErrorHalfProjectionIntegration.h
\brief Simpson Integrator based on error value
\author Leonardo Quatrin Campagnolo
*/

#ifndef SIMPSONIEHPEHINTEGRATOR_H
#define SIMPSONIEHPEHINTEGRATOR_H

#include <cstdlib>
#include <math/Vector3.h>
#include <math/Vector4.h>
#include <math/MGeometry.h>

#include <volrend/Volume.h>
#include <volrend/TransferFunction.h>
#include "defines.h"

#include <algorithm>

class VolumeEvaluator;

class AdaptiveSimpsonIEHPEHIntegrator
{
public:
  static int s_IntegrativeExternalTimes;
  static int s_IntegrativeInternalTimes;

  static int s_RecalculateIntegrativeExternalTimes;
  static int s_RecalculateIntegrativeInternalTimes;

public:
  AdaptiveSimpsonIEHPEHIntegrator (VolumeEvaluator* veva);
  ~AdaptiveSimpsonIEHPEHIntegrator ();

  void Reset ();

  void Init (lqc::Vector3d minp, lqc::Vector3d maxp, vr::Volume* vol, vr::TransferFunction* tf);
  void Integrate (double s0, double s1, double tol, double h0);
  
  lqc::Vector4d color;

  double m_clc_proj_newinternalprojection;
  double m_clc_proj_internal_error;
  lqc::Vector4d m_clc_proj_external_error;

protected:
  int m_integrate_method;
  
  double InternalIntegral (double s, double h, double error, double* hproj);
  void ExternalIntegral (double s, double h, double error);
  
  bool ColorErrorEvalFunc (lqc::Vector4d a, lqc::Vector4d b, double tol);
  double IntegrateInternalInterval (double s, double h, double tol);

  lqc::Vector4d ExtenalEvaluation (double p_d, lqc::Vector4d C, double* pre_alpha = NULL);
  lqc::Vector4d ExtenalEvaluationMiddle (double p_d, lqc::Vector4d C, lqc::Vector4d Cmid, double* pre_alpha = NULL);
  bool IntegrateExternalInterval (double s, double h, double tol, double* pre_alpha, bool minorstep, lqc::Vector4d* color_ret, lqc::Vector4d f_left[]);

  lqc::Vector4d GetFromTransferFunction (double p_d);

  double AdaptiveExternalIntegration (double s, double h, double error);

  void Test_ExternalIntegral (double s, double h, double error);
  double Test_AdaptiveExternalIntegration (double s, double h, double error);

private:
  double MaxExternalError (lqc::Vector4d err)
  {
    return std::max (std::max (err.x, err.y), std::max (err.z, err.w));
  }

  double pre_integrated;
  double minpost;
  lqc::Vector4d Cminpost;

  VolumeEvaluator* volume_evaluator;
  vr::Volume* volume;
  vr::TransferFunction* transfer_function;

  lqc::Vector3d minpos;
  lqc::Vector3d maxpos;
  lqc::Vector3d normalized_step;

  lqc::Vector4d external_error_aux;




  lqc::Vector4d lastValue;
  lqc::Vector4d f_left[5], f_right[5];
};

#endif