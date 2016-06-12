#ifndef GROUNDTRUTHRIEMMAN_HPP
#define GROUNDTRUTHRIEMMAN_HPP

#include <volrend/Volume.h>
#include <volrend/TransferFunction.h>
#include "defines.h"

class VolumeEvaluator;

class RiemannSummation
{
public:
  RiemannSummation (VolumeEvaluator* veva, vr::Volume* vol, vr::TransferFunction* tf);
  ~RiemannSummation ();

  void Composite (lqc::Vector4d* color, lqc::Vector3d pos, double stepdistance);

  lqc::Vector4d color;
  double opacity;

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
  std::ofstream* file;
  double s;
#endif

protected:
private:
  VolumeEvaluator* volume_evaluator;
  vr::Volume* volume;
  vr::TransferFunction* transfer_function;
};

class RiemannManualSummation
{
public:
  RiemannManualSummation (VolumeEvaluator* veva);
  ~RiemannManualSummation ();

  void Init (vr::Volume* vol, vr::TransferFunction* tf, lqc::Vector3d minp, lqc::Vector3d n_step);
  lqc::Vector4d EstipulateSum (double s0, double s1, double stepdistance);
  lqc::Vector4d GetFromTransferFunction (double p_d);
  void Composite ();

  lqc::Vector3d norm_step;
  lqc::Vector3d minpos;

  lqc::Vector4d aux_color;
  double aux_opacity;

  lqc::Vector4d color;
  double opacity;
protected:
private:
  VolumeEvaluator* volume_evaluator;
  vr::Volume* volume;
  vr::TransferFunction* transfer_function;
};

class RiemannExpManualSummation
{
public:
  RiemannExpManualSummation (VolumeEvaluator* veva);
  ~RiemannExpManualSummation ();

  void Init (vr::Volume* vol, vr::TransferFunction* tf, lqc::Vector3d minp, lqc::Vector3d n_step);
  lqc::Vector4d EstipulateSum (double s0, double s1, double stepdistance);
  void SumAndComposite (lqc::Vector4d* color, lqc::Vector3d pos, double stepdistance);
  lqc::Vector4d GetFromTransferFunction (double p_d);
  void Composite ();

  lqc::Vector3d norm_step;
  lqc::Vector3d minpos;

  lqc::Vector4d aux_color;
  double aux_opacity;
  double pre_integrated2;

  lqc::Vector4d color;
  double pre_integrated;
protected:
private:
  VolumeEvaluator* volume_evaluator;
  vr::Volume* volume;
  vr::TransferFunction* transfer_function;
};

#endif