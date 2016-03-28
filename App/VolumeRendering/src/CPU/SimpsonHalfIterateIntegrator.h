/*!
\file SimpsonHalfIterateIntegrator.h
\brief Simpson Integrator based on error value
\author Leonardo Quatrin Campagnolo
*/

#ifndef SIMPSON_HALF_ITERATE_INTEGRATOR_H
#define SIMPSON_HALF_ITERATE_INTEGRATOR_H

#include "SimpsonIntegrator.h"

#include <cstdlib>
#include <math/Vector3.h>
#include <math/Vector4.h>
#include <math/MGeometry.h>

#include <volrend/Volume.h>
#include <volrend/TransferFunction.h>
#include "defines.h"

#include <algorithm>

class VolumeEvaluator;

class SimpsonHalfIterateIntegrator : public SimpsonIntegrator
{
public:
  SimpsonHalfIterateIntegrator (VolumeEvaluator* veva);
  ~SimpsonHalfIterateIntegrator ();

  void Reset ();

  void Init (lqc::Vector3d minp, lqc::Vector3d maxp, vr::Volume* vol, vr::TransferFunction* tf);
  void IntegrateSimple (double s0, double s1, double tol, double h0, double hmint, double hmaxt);
  void CoupledIntegration (double s0, double s1, double tol, double h0, double hmint, double hmaxt);
  void IntegrateSimpleExtStep (double s0, double s1, double tol, double h0, double hmint, double hmaxt);
  void IntegrateComplexExtStep (double s0, double s1, double tol, double h0, double hmint, double hmaxt);
  void IntegrateExp (double s0, double s1, double tol, double h0, double hmint, double hmaxt);

  void IntegrateSeparated (double s0, double s1, double tol, double h0, double hmint, double hmaxt);
  void DecoupledIntegration (double s0, double s1, double tol, double h0, double hmin, double hmax);
  void IntegrateScount (double s0, double s1, double tol, double h0, double hmin, double hmax);

  void PrintStepsEvaluation ();

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
    std::ofstream* file_ext;
    std::ofstream* file_int;

    void PrintExternalStepSize (double h);
    void PrintInternalStepSize (double h);
    void PrintInternalColor ();
#endif
protected:

private:
  #ifdef COMPUTE_STEPS_ALONG_EVALUATION
    int m_steps_evaluation[11];
  #endif
};

#endif