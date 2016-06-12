#include "SimpsonIntegrator.h"
#include "VolumeEvaluator.h"
#include <cmath>

SimpsonIntegrator::SimpsonIntegrator (VolumeEvaluator* veva)
{
  volume_evaluator = veva;
}

SimpsonIntegrator::~SimpsonIntegrator ()
{}

lqc::Vector4d SimpsonIntegrator::ExternalEvaluationAnchor ()
{
  double alphachannel = anchor_color.w*(exp (-(pre_integrated)));
  return lqc::Vector4d (alphachannel * anchor_color.x, alphachannel * anchor_color.y, alphachannel * anchor_color.z, alphachannel);
}

lqc::Vector4d SimpsonIntegrator::ExternalEvaluation (double p_d, lqc::Vector4d C, lqc::Vector4d Cmid)
{
  double alphachannel = C.w*(exp (-(pre_integrated + (((p_d - anchor) / 6.0) * (anchor_color.w + 4.0 * Cmid.w + C.w)))));
  return lqc::Vector4d (alphachannel * C.x, alphachannel * C.y, alphachannel * C.z, alphachannel);
}

lqc::Vector4d SimpsonIntegrator::ExternalEvaluation (double p_d, lqc::Vector4d C, double Cmid_w)
{
  double alphachannel = C.w*(exp (-(pre_integrated + (((p_d - anchor) / 6.0) * (anchor_color.w + 4.0 * Cmid_w + C.w)))));
  return lqc::Vector4d (alphachannel * C.x, alphachannel * C.y, alphachannel * C.z, alphachannel);
}

//exp = 1 + x + x²/2 + x³/6
lqc::Vector4d SimpsonIntegrator::ExternalEvaluationApprox (double p_d, lqc::Vector4d C, lqc::Vector4d Cmid)
{
  double x = -((p_d - anchor) / 6.0) * (anchor_color.w + 4.0 * Cmid.w + C.w);
  double alphachannel = C.w*pre_integrated*(1.0 + x + (x*x) / 2.0 + (x*x*x) / 6.0);
  return lqc::Vector4d (alphachannel * C.x, alphachannel * C.y, alphachannel * C.z, alphachannel);
}

lqc::Vector4d SimpsonIntegrator::ExternalEvaluation (double p_d, lqc::Vector4d C, lqc::Vector4d Cmid, double* pre_alpha)
{
  double alpha = 0.0;
  double h = p_d - anchor;
  if (h != 0)
    alpha = (h / 6.0) * (anchor_color.w + 4.0 * Cmid.w + C.w);

  double innerint = exp (-(pre_integrated + alpha));

  if (pre_alpha)
    (*pre_alpha) = pre_integrated + alpha;

  double alphachannel = C.w*innerint;
  return lqc::Vector4d (alphachannel * C.x, alphachannel * C.y, alphachannel * C.z, alphachannel);
}

lqc::Vector4d SimpsonIntegrator::ExternalEvaluationInnerPreCalculated (double p_d, lqc::Vector4d C, double inner)
{
  double alphachannel = C.w*exp (-(pre_integrated + inner));
  return lqc::Vector4d (alphachannel * C.x, alphachannel * C.y, alphachannel * C.z, alphachannel);
}

lqc::Vector4d SimpsonIntegrator::GetFromTransferFunction (double p_d)
{
  lqc::Vector4d ret;
  lqc::Vector3d p = minpos + p_d * normalized_step;
  if (!volume) ret = lqc::Vector4d (0.0);
  else ret = volume_evaluator->TransferFunction (volume_evaluator->GetValueFromVolume (volume, lqc::Vector3f (p.x, p.y, p.z)));
  return ret;
}

void SimpsonIntegrator::Init (lqc::Vector3d minp, lqc::Vector3d maxp, vr::Volume* vol, vr::TransferFunction* tf)
{
  volume = vol;
  transfer_function = tf;

  minpos = minp;
  maxpos = maxp;
  normalized_step = lqc::Vector3d::Normalize (maxpos - minpos);

  pre_integrated = 0.0;
  color = lqc::Vector4d (0);
}