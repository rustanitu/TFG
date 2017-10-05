#ifndef GROUNDTRUTHRIEMMAN_HPP
#define GROUNDTRUTHRIEMMAN_HPP

#include <atfg/Volume.h>
#include <atfg/TransferFunction.h>
#include "defines.h"

class VolumeEvaluator;

class RiemannSummation
{
public:
  RiemannSummation (VolumeEvaluator* veva, vr::Volume* vol, vr::TransferFunction* tf);
  ~RiemannSummation ();

  void Composite (glm::dvec4* color, glm::dvec3 pos, double stepdistance);

  glm::dvec4 color;
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

  void Init (vr::Volume* vol, vr::TransferFunction* tf, glm::dvec3 minp, glm::dvec3 n_step);
  glm::dvec4 EstipulateSum (double s0, double s1, double stepdistance);
  glm::dvec4 GetFromTransferFunction (double p_d);
  void Composite ();

  glm::dvec3 norm_step;
  glm::dvec3 minpos;

  glm::dvec4 aux_color;
  double aux_opacity;

  glm::dvec4 color;
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

  void Init (vr::Volume* vol, vr::TransferFunction* tf, glm::dvec3 minp, glm::dvec3 n_step);
  glm::dvec4 EstipulateSum (double s0, double s1, double stepdistance);
  void SumAndComposite (glm::dvec4* color, glm::dvec3 pos, double stepdistance);
  glm::dvec4 GetFromTransferFunction (double p_d);
  void Composite ();

  glm::dvec3 norm_step;
  glm::dvec3 minpos;

  glm::dvec4 aux_color;
  double aux_opacity;
  double pre_integrated2;

  glm::dvec4 color;
  double pre_integrated;
protected:
private:
  VolumeEvaluator* volume_evaluator;
  vr::Volume* volume;
  vr::TransferFunction* transfer_function;
};

#endif