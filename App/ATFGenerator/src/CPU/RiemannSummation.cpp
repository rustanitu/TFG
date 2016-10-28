#include "RiemannSummation.hpp"

#include <volrend/ScalarField.h>
#include <volrend/TransferFunction.h>
#include "defines.h"
#include "VolumeEvaluator.h"

RiemannSummation::RiemannSummation(VolumeEvaluator* veva, vr::Volume* vol, vr::TransferFunction* tf)
{
  volume_evaluator = veva;
  volume = vol;
  transfer_function = tf;

  opacity = 1.0;
  color = lqc::Vector4d (0);

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
  s = 0;
  file = NULL;
#endif

}

RiemannSummation::~RiemannSummation () {}

void RiemannSummation::Composite (lqc::Vector4d* color, lqc::Vector3d pos, double stepdistance)
{
  lqc::Vector3f p = lqc::Vector3f (pos.x, pos.y, pos.z);
  float value = volume_evaluator->GetValueFromVolume(volume, p);
  lqc::Vector4d src;
  lqc::Vector4d tfsrc = volume_evaluator->TransferFunction (value);
  src = lqc::Vector4d (tfsrc.x, tfsrc.y, tfsrc.z, tfsrc.w);

  double d = stepdistance;

  (*color).x += src.x*src.w*d * opacity;
  (*color).y += src.y*src.w*d * opacity;
  (*color).z += src.z*src.w*d * opacity;
  (*color).w += src.w*d       * opacity;

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
  if (file)
  {
    s += d;
    (*file) << std::setprecision (30) << s << '\t'
      << std::setprecision (30) << d << '\t'
      << std::setprecision (30) << (*color).x << '\t'
      << std::setprecision (30) << (*color).y << '\t'
      << std::setprecision (30) << (*color).z << '\t'
      << std::setprecision (30) << (*color).w << '\n';
  }
#endif

  opacity *= (1.0 - src.w*d);
}


RiemannManualSummation::RiemannManualSummation (VolumeEvaluator* veva)
{
  volume_evaluator = veva;
}

RiemannManualSummation::~RiemannManualSummation () {}

void RiemannManualSummation::Init(vr::Volume* vol, vr::TransferFunction* tf, lqc::Vector3d minp, lqc::Vector3d n_step)
{
  volume = vol;
  transfer_function = tf;

  opacity = 1.0;
  color = lqc::Vector4d (0);

  norm_step = n_step;
  minpos = minp;
}

lqc::Vector4d RiemannManualSummation::EstipulateSum (double s0, double s1, double stepdistance)
{
  aux_color = lqc::Vector4d (0);
  aux_opacity = opacity;

  double s = s0;
  lqc::Vector4d src;
  double h = std::min (stepdistance, fabs (s1 - s0));

  while (s < s1)
  {
    h = std::min (h, s1 - s);

    src = GetFromTransferFunction (s + h);

    aux_opacity *= (1.0 - src.w*h);

    aux_color.x += src.x*src.w*h * aux_opacity;
    aux_color.y += src.y*src.w*h * aux_opacity;
    aux_color.z += src.z*src.w*h * aux_opacity;
    aux_color.w += src.w*h       * aux_opacity;

    s = s + h;
  }

  return aux_color;
}

lqc::Vector4d RiemannManualSummation::GetFromTransferFunction (double p_d)
{
  lqc::Vector4d ret;
  lqc::Vector3d p = minpos + p_d * norm_step;
  if (!transfer_function || !volume) ret = lqc::Vector4d (0.0);
  else ret = transfer_function->Get(volume_evaluator->GetValueFromVolume(volume, lqc::Vector3f(p.x, p.y, p.z)));
  return ret;
}

void RiemannManualSummation::Composite ()
{
  color = aux_color;
  opacity = aux_opacity;
}

RiemannExpManualSummation::RiemannExpManualSummation (VolumeEvaluator* veva)
{
  volume_evaluator = veva;
}

RiemannExpManualSummation::~RiemannExpManualSummation () {}

void RiemannExpManualSummation::Init(vr::Volume* vol, vr::TransferFunction* tf, lqc::Vector3d minp, lqc::Vector3d n_step)
{
  volume = vol;
  transfer_function = tf;

  pre_integrated = 0.0;
  color = lqc::Vector4d (0);

  norm_step = n_step;
  minpos = minp;
  pre_integrated2 = 1.0;
}

lqc::Vector4d RiemannExpManualSummation::EstipulateSum (double s0, double s1, double stepdistance)
{
  aux_color = lqc::Vector4d (0);
  aux_opacity = 0;

  double s = s0;
  lqc::Vector4d src;
  double h = std::min (stepdistance, fabs (s1 - s0));

  while (s < s1)
  {
    h = std::min (h, s1 - s);

    src = GetFromTransferFunction (s + h);

    aux_opacity += src.w*h;

    double alphachannel = src.w*h*exp (-(aux_opacity + pre_integrated));
    aux_color.x += src.x*alphachannel;
    aux_color.y += src.y*alphachannel;
    aux_color.z += src.z*alphachannel;
    aux_color.w += alphachannel;

    s = s + h;
  }

  return aux_color;
}

lqc::Vector4d RiemannExpManualSummation::GetFromTransferFunction (double p_d)
{
  lqc::Vector4d ret;
  lqc::Vector3d p = minpos + p_d * norm_step;
  if (!transfer_function || !volume) ret = lqc::Vector4d (0.0);
  else ret = transfer_function->Get(volume_evaluator->GetValueFromVolume(volume, lqc::Vector3f(p.x, p.y, p.z)));
  return ret;
}

void RiemannExpManualSummation::Composite ()
{
  color += aux_color;
  pre_integrated += aux_opacity;
}

void RiemannExpManualSummation::SumAndComposite (lqc::Vector4d* color, lqc::Vector3d pos, double stepdistance)
{
  lqc::Vector4d src = volume_evaluator->TransferFunction(volume_evaluator->GetValueFromVolume(volume, lqc::Vector3f(pos.x, pos.y, pos.z)));

  double opacity = exp (-(src.w*stepdistance));

  pre_integrated2 *= opacity;

  (*color).x += src.x * src.w * pre_integrated2 * stepdistance;
  (*color).y += src.y * src.w * pre_integrated2 * stepdistance;
  (*color).z += src.z * src.w * pre_integrated2 * stepdistance;
  (*color).w += src.w * pre_integrated2 * stepdistance;
}

