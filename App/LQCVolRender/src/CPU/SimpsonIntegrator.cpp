#include "SimpsonIntegrator.h"
#include "VolumeEvaluator.h"
#include <cmath>

SimpsonIntegrator::SimpsonIntegrator (VolumeEvaluator* veva)
{
  volume_evaluator = veva;
}

SimpsonIntegrator::~SimpsonIntegrator ()
{}

glm::dvec4 SimpsonIntegrator::ExternalEvaluationAnchor ()
{
  double alphachannel = anchor_color.w*(exp (-(pre_integrated)));
  return glm::dvec4 (alphachannel * anchor_color.x, alphachannel * anchor_color.y, alphachannel * anchor_color.z, alphachannel);
}

glm::dvec4 SimpsonIntegrator::ExternalEvaluation (double p_d, glm::dvec4 C, glm::dvec4 Cmid)
{
  double alphachannel = C.w*(exp (-(pre_integrated + (((p_d - anchor) / 6.0) * (anchor_color.w + 4.0 * Cmid.w + C.w)))));
  return glm::dvec4 (alphachannel * C.x, alphachannel * C.y, alphachannel * C.z, alphachannel);
}

glm::dvec4 SimpsonIntegrator::ExternalEvaluation (double p_d, glm::dvec4 C, double Cmid_w)
{
  double alphachannel = C.w*(exp (-(pre_integrated + (((p_d - anchor) / 6.0) * (anchor_color.w + 4.0 * Cmid_w + C.w)))));
  return glm::dvec4 (alphachannel * C.x, alphachannel * C.y, alphachannel * C.z, alphachannel);
}

//exp = 1 + x + x²/2 + x³/6
glm::dvec4 SimpsonIntegrator::ExternalEvaluationApprox (double p_d, glm::dvec4 C, glm::dvec4 Cmid)
{
  double x = -((p_d - anchor) / 6.0) * (anchor_color.w + 4.0 * Cmid.w + C.w);
  double alphachannel = C.w*pre_integrated*(1.0 + x + (x*x) / 2.0 + (x*x*x) / 6.0);
  return glm::dvec4 (alphachannel * C.x, alphachannel * C.y, alphachannel * C.z, alphachannel);
}

glm::dvec4 SimpsonIntegrator::ExternalEvaluation (double p_d, glm::dvec4 C, glm::dvec4 Cmid, double* pre_alpha)
{
  double alpha = 0.0;
  double h = p_d - anchor;
  if (h != 0)
    alpha = (h / 6.0) * (anchor_color.w + 4.0 * Cmid.w + C.w);

  double innerint = exp (-(pre_integrated + alpha));

  if (pre_alpha)
    (*pre_alpha) = pre_integrated + alpha;

  double alphachannel = C.w*innerint;
  return glm::dvec4 (alphachannel * C.x, alphachannel * C.y, alphachannel * C.z, alphachannel);
}

glm::dvec4 SimpsonIntegrator::ExternalEvaluationInnerPreCalculated (double p_d, glm::dvec4 C, double inner)
{
  double alphachannel = C.w*exp (-(pre_integrated + inner));
  return glm::dvec4 (alphachannel * C.x, alphachannel * C.y, alphachannel * C.z, alphachannel);
}

glm::dvec4 SimpsonIntegrator::GetFromTransferFunction (double p_d)
{
  glm::dvec4 ret;
  glm::dvec3 p = minpos + p_d * normalized_step;
  if (!volume) ret = glm::dvec4 (0.0);
  else ret = volume_evaluator->TransferFunction(volume_evaluator->GetValueFromVolume(volume, lqc::Vector3f(p.x, p.y, p.z)));
  return ret;
}

void SimpsonIntegrator::Init(glm::dvec3 minp, glm::dvec3 maxp, vr::Volume* vol, vr::TransferFunction* tf)
{
  volume = vol;
  transfer_function = tf;

  minpos = minp;
  maxpos = maxp;
  normalized_step = glm::normalize (maxpos - minpos);

  pre_integrated = 0.0;
  color = glm::dvec4 (0);
}