#ifndef SIMPSONRULE_HPP
#define SIMPSONRULE_HPP

#include <volrend/Volume.h>
#include <volrend/TransferFunction.h>
#include "defines.h"

class VolumeEvaluator;

class SimpsonRuleSummation
{
public:
  SimpsonRuleSummation (VolumeEvaluator* veva, vr::Volume* vol, vr::TransferFunction* tf);
  ~SimpsonRuleSummation ();

  void Composite (lqc::Vector4d* color, double stepdistance, lqc::Vector3d p1, lqc::Vector3d p2, lqc::Vector3d p3);

  lqc::Vector4d color;
  double opacity;
  double exp_opacity;
protected:
private:
  VolumeEvaluator* volume_evaluator;
  vr::Volume* volume;
  vr::TransferFunction* transfer_function;
};

#endif