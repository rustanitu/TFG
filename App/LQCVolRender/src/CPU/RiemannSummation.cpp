#include "RiemannSummation.hpp"

#include <atfg/ScalarField.h>
#include <atfg/TransferFunction.h>
#include "defines.h"
#include "VolumeEvaluator.h"

RiemannSummation::RiemannSummation(VolumeEvaluator* veva, vr::Volume* vol, vr::TransferFunction* tf)
{
  volume_evaluator = veva;
  volume = vol;
  transfer_function = tf;

  opacity = 1.0;
  color = glm::dvec4 (0);

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
  s = 0;
  file = NULL;
#endif

}

RiemannSummation::~RiemannSummation () {}

void RiemannSummation::Composite (glm::dvec4* color, glm::dvec3 pos, double stepdistance)
{
  lqc::Vector3f p = lqc::Vector3f (pos.x, pos.y, pos.z);
  float value = volume_evaluator->GetValueFromVolume(volume, p);
  glm::dvec4 src;
  glm::dvec4 tfsrc = volume_evaluator->TransferFunction (value);
  src = glm::dvec4 (tfsrc.x, tfsrc.y, tfsrc.z, tfsrc.w);

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

void RiemannManualSummation::Init(vr::Volume* vol, vr::TransferFunction* tf, glm::dvec3 minp, glm::dvec3 n_step)
{
  volume = vol;
  transfer_function = tf;

  opacity = 1.0;
  color = glm::dvec4 (0);

  norm_step = n_step;
  minpos = minp;
}

glm::dvec4 RiemannManualSummation::EstipulateSum (double s0, double s1, double stepdistance)
{
  aux_color = glm::dvec4 (0);
  aux_opacity = opacity;

  double s = s0;
  glm::dvec4 src;
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

glm::dvec4 RiemannManualSummation::GetFromTransferFunction (double p_d)
{
  glm::dvec4 ret;
  glm::dvec3 p = minpos + p_d * norm_step;
  if (!transfer_function || !volume) ret = glm::dvec4 (0.0);
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

void RiemannExpManualSummation::Init(vr::Volume* vol, vr::TransferFunction* tf, glm::dvec3 minp, glm::dvec3 n_step)
{
  volume = vol;
  transfer_function = tf;

  pre_integrated = 0.0;
  color = glm::dvec4 (0);

  norm_step = n_step;
  minpos = minp;
  pre_integrated2 = 1.0;
}

glm::dvec4 RiemannExpManualSummation::EstipulateSum (double s0, double s1, double stepdistance)
{
  aux_color = glm::dvec4 (0);
  aux_opacity = 0;

  double s = s0;
  glm::dvec4 src;
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

glm::dvec4 RiemannExpManualSummation::GetFromTransferFunction (double p_d)
{
  glm::dvec4 ret;
  glm::dvec3 p = minpos + p_d * norm_step;
  if (!transfer_function || !volume) ret = glm::dvec4 (0.0);
  else ret = transfer_function->Get(volume_evaluator->GetValueFromVolume(volume, lqc::Vector3f(p.x, p.y, p.z)));
  return ret;
}

void RiemannExpManualSummation::Composite ()
{
  color += aux_color;
  pre_integrated += aux_opacity;
}

void RiemannExpManualSummation::SumAndComposite (glm::dvec4* color, glm::dvec3 pos, double stepdistance)
{
  glm::dvec4 src = volume_evaluator->TransferFunction(volume_evaluator->GetValueFromVolume(volume, lqc::Vector3f(pos.x, pos.y, pos.z)));

  double opacity = exp (-(src.w*stepdistance));

  pre_integrated2 *= opacity;

  (*color).x += src.x * src.w * pre_integrated2 * stepdistance;
  (*color).y += src.y * src.w * pre_integrated2 * stepdistance;
  (*color).z += src.z * src.w * pre_integrated2 * stepdistance;
  (*color).w += src.w * pre_integrated2 * stepdistance;
}

