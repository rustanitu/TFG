#ifndef SIMPSONRULE_HPP
#define SIMPSONRULE_HPP

#include <atfg/Volume.h>
#include <atfg/TransferFunction.h>
#include "defines.h"

class VolumeEvaluator;

class SimpsonRuleSummation
{
public:
  SimpsonRuleSummation (VolumeEvaluator* veva, vr::Volume* vol, vr::TransferFunction* tf);
  ~SimpsonRuleSummation ();

  void Composite (glm::dvec4* color, double stepdistance, glm::dvec3 p1, glm::dvec3 p2, glm::dvec3 p3);

  glm::dvec4 color;
  double opacity;
  double exp_opacity;
protected:
private:
  VolumeEvaluator* volume_evaluator;
  vr::Volume* volume;
  vr::TransferFunction* transfer_function;
};

#endif