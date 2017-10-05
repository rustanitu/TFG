#include "SimpsonRule.hpp"

#include <atfg/Volume.h>
#include <atfg/TransferFunction.h>
#include "defines.h"
#include "VolumeEvaluator.h"

SimpsonRuleSummation::SimpsonRuleSummation (VolumeEvaluator* veva, vr::Volume* vol, vr::TransferFunction* tf)
{
  volume_evaluator = veva;
  volume = vol;
  transfer_function = tf;

  opacity = 0.0;
  color = glm::dvec4 (0);
  exp_opacity = 1.0;
}

SimpsonRuleSummation::~SimpsonRuleSummation () {}

void SimpsonRuleSummation::Composite (glm::dvec4* color, double stepdistance, glm::dvec3 p1, glm::dvec3 p2, glm::dvec3 p3)
{
  glm::dvec4 tf_a = volume_evaluator->TransferFunction (volume_evaluator->GetValueFromVolume (volume, glm::vec3 (p1.x, p1.y, p1.z)));
  glm::dvec4 tf_c = volume_evaluator->TransferFunction (volume_evaluator->GetValueFromVolume (volume, glm::vec3 (p2.x, p2.y, p2.z)));
  glm::dvec4 tf_b = volume_evaluator->TransferFunction (volume_evaluator->GetValueFromVolume (volume, glm::vec3 (p3.x, p3.y, p3.z)));
  glm::dvec4 tf_d = volume_evaluator->TransferFunction (volume_evaluator->GetValueFromVolume (volume, glm::vec3 ((p1.x + p2.x) * 0.5, (p1.y + p2.y) * 0.5, (p1.z + p2.z) * 0.5)));
  glm::dvec4 tf_e = volume_evaluator->TransferFunction (volume_evaluator->GetValueFromVolume (volume, glm::vec3 ((p3.x + p2.x) * 0.5, (p3.y + p2.y) * 0.5, (p3.z + p2.z) * 0.5)));
  double d = stepdistance;

  double a_channel = tf_a.w * exp_opacity;
  glm::dvec4 F_a = glm::dvec4 (tf_a.x*a_channel, tf_a.y*a_channel, tf_a.z*a_channel, a_channel);

  opacity += (d / 12.0)*(tf_a.w + 4.0 * tf_d.w + tf_c.w);
  a_channel = tf_c.w * std::exp (-opacity);
  glm::dvec4 F_c = glm::dvec4 (tf_c.x*a_channel, tf_c.y*a_channel, tf_c.z*a_channel, a_channel);

  opacity += (d / 12.0)*(tf_c.w + 4.0 * tf_e.w + tf_b.w);
  exp_opacity = std::exp (-opacity);
  a_channel = tf_b.w * exp_opacity;
  glm::dvec4 F_b = glm::dvec4 (tf_b.x*a_channel, tf_b.y*a_channel, tf_b.z*a_channel, a_channel);

  (*color) += (d / 6.0) * (F_a + 4.0 * F_c + F_b);
}