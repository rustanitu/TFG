#include "SimpsonRule.hpp"

#include <volrend/Volume.h>
#include <volrend/TransferFunction.h>
#include "defines.h"
#include "VolumeEvaluator.h"

SimpsonRuleSummation::SimpsonRuleSummation (VolumeEvaluator* veva, vr::Volume* vol, vr::TransferFunction* tf)
{
  volume_evaluator = veva;
  volume = vol;
  transfer_function = tf;

  opacity = 0.0;
  color = lqc::Vector4d (0);
  exp_opacity = 1.0;
}

SimpsonRuleSummation::~SimpsonRuleSummation () {}

void SimpsonRuleSummation::Composite (lqc::Vector4d* color, double stepdistance, lqc::Vector3d p1, lqc::Vector3d p2, lqc::Vector3d p3)
{
  lqc::Vector4d tf_a = volume_evaluator->TransferFunction (volume_evaluator->GetValueFromVolume (volume, lqc::Vector3f (p1.x, p1.y, p1.z)));
  lqc::Vector4d tf_c = volume_evaluator->TransferFunction (volume_evaluator->GetValueFromVolume (volume, lqc::Vector3f (p2.x, p2.y, p2.z)));
  lqc::Vector4d tf_b = volume_evaluator->TransferFunction (volume_evaluator->GetValueFromVolume (volume, lqc::Vector3f (p3.x, p3.y, p3.z)));
  lqc::Vector4d tf_d = volume_evaluator->TransferFunction (volume_evaluator->GetValueFromVolume (volume, lqc::Vector3f ((p1.x + p2.x) * 0.5, (p1.y + p2.y) * 0.5, (p1.z + p2.z) * 0.5)));
  lqc::Vector4d tf_e = volume_evaluator->TransferFunction (volume_evaluator->GetValueFromVolume (volume, lqc::Vector3f ((p3.x + p2.x) * 0.5, (p3.y + p2.y) * 0.5, (p3.z + p2.z) * 0.5)));
  double d = stepdistance;

  double a_channel = tf_a.w * exp_opacity;
  lqc::Vector4d F_a = lqc::Vector4d (tf_a.x*a_channel, tf_a.y*a_channel, tf_a.z*a_channel, a_channel);

  opacity += (d / 12.0)*(tf_a.w + 4.0 * tf_d.w + tf_c.w);
  a_channel = tf_c.w * std::exp (-opacity);
  lqc::Vector4d F_c = lqc::Vector4d (tf_c.x*a_channel, tf_c.y*a_channel, tf_c.z*a_channel, a_channel);

  opacity += (d / 12.0)*(tf_c.w + 4.0 * tf_e.w + tf_b.w);
  exp_opacity = std::exp (-opacity);
  a_channel = tf_b.w * exp_opacity;
  lqc::Vector4d F_b = lqc::Vector4d (tf_b.x*a_channel, tf_b.y*a_channel, tf_b.z*a_channel, a_channel);

  (*color) += (d / 6.0) * (F_a + 4.0 * F_c + F_b);
}