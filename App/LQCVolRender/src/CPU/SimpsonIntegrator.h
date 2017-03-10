/*!
\file SimpsonIntegrator.h
\brief Simpson Integrator base class
\author Leonardo Quatrin Campagnolo
*/

#ifndef SIMPSON_INTEGRATOR_H
#define SIMPSON_INTEGRATOR_H

#include <atfg/Volume.h>
#include <atfg/TransferFunction.h>

#include <math/Vector3.h>
#include <math/Vector4.h>

class VolumeEvaluator;

class SimpsonIntegrator
{
public:
  SimpsonIntegrator (VolumeEvaluator* veva);
  ~SimpsonIntegrator ();

  glm::dvec4 color;

protected:
  void Init (glm::dvec3 minp, glm::dvec3 maxp, vr::Volume* vol, vr::TransferFunction* tf);
  glm::dvec4 ExternalEvaluationAnchor ();
  glm::dvec4 ExternalEvaluation (double p_d, glm::dvec4 C, glm::dvec4 Cmid);
  glm::dvec4 ExternalEvaluation (double p_d, glm::dvec4 C, double Cmid_w);
  glm::dvec4 ExternalEvaluation (double p_d, glm::dvec4 C, glm::dvec4 Cmid, double* pre_alpha);
  glm::dvec4 ExternalEvaluationInnerPreCalculated (double p_d, glm::dvec4 C, double inner);
  glm::dvec4 ExternalEvaluationApprox (double p_d, glm::dvec4 C, glm::dvec4 Cmid);
  glm::dvec4 GetFromTransferFunction (double p_d);

  double anchor;
  glm::dvec4 anchor_color;
  double pre_integrated;

  VolumeEvaluator* volume_evaluator;
  vr::Volume* volume;
  vr::TransferFunction* transfer_function;

  glm::dvec3 minpos;
  glm::dvec3 maxpos;
  glm::dvec3 normalized_step;

private:
};

#endif