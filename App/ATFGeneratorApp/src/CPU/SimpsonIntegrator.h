/*!
\file SimpsonIntegrator.h
\brief Simpson Integrator base class
\author Leonardo Quatrin Campagnolo
*/

#ifndef SIMPSON_INTEGRATOR_H
#define SIMPSON_INTEGRATOR_H

#include <volrend/Volume.h>
#include <volrend/TransferFunction.h>

#include <math/Vector3.h>
#include <math/Vector4.h>

class VolumeEvaluator;

class SimpsonIntegrator
{
public:
  SimpsonIntegrator (VolumeEvaluator* veva);
  ~SimpsonIntegrator ();

  lqc::Vector4d color;

protected:
  void Init (lqc::Vector3d minp, lqc::Vector3d maxp, vr::Volume* vol, vr::TransferFunction* tf);
  lqc::Vector4d ExternalEvaluationAnchor ();
  lqc::Vector4d ExternalEvaluation (double p_d, lqc::Vector4d C, lqc::Vector4d Cmid);
  lqc::Vector4d ExternalEvaluation (double p_d, lqc::Vector4d C, double Cmid_w);
  lqc::Vector4d ExternalEvaluation (double p_d, lqc::Vector4d C, lqc::Vector4d Cmid, double* pre_alpha);
  lqc::Vector4d ExternalEvaluationInnerPreCalculated (double p_d, lqc::Vector4d C, double inner);
  lqc::Vector4d ExternalEvaluationApprox (double p_d, lqc::Vector4d C, lqc::Vector4d Cmid);
  lqc::Vector4d GetFromTransferFunction (double p_d);

  double anchor;
  lqc::Vector4d anchor_color;
  double pre_integrated;

  VolumeEvaluator* volume_evaluator;
  vr::Volume* volume;
  vr::TransferFunction* transfer_function;

  lqc::Vector3d minpos;
  lqc::Vector3d maxpos;
  lqc::Vector3d normalized_step;

private:
};

#endif