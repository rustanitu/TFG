/*!
\file SimpsonHalfIteratorRiemannError.h
\brief Simpson Integrator based on error value
\author Leonardo Quatrin Campagnolo
*/

#ifndef SIMPSON_HALF_ITERATOR_RIEMANN_ERROR_H
#define SIMPSON_HALF_ITERATOR_RIEMANN_ERROR_H

#include "SimpsonIntegrator.h"

#include <cstdlib>
#include <math/Vector3.h>
#include <math/Vector4.h>
#include <math/MGeometry.h>

#include <volrend/Volume.h>
#include <volrend/TransferFunction.h>
#include "defines.h"

#include "RiemannSummation.hpp"


#include <algorithm>

class VolumeEvaluator;

class SimpsonHalfIteratorRiemannError : public SimpsonIntegrator
{
public:
  SimpsonHalfIteratorRiemannError (VolumeEvaluator* veva);
  ~SimpsonHalfIteratorRiemannError ();

  void Reset ();

  void Init (glm::dvec3 minp, glm::dvec3 maxp, vr::Volume* vol, vr::TransferFunction* tf);
  void Integrate (double s0, double s1, double tol, double h0, double hmint, double hmaxt);

  void PrintStepsEvaluation ();

  double m_clc_proj_newinternalprojection;
  double m_clc_proj_internal_error;
  glm::dvec4 m_clc_proj_external_error;

protected:
  double InternalIntervalError (double s, double h);
  double InternalIntegral (double s, double h, double error);

  bool ColorErrorEvalFunc (glm::dvec4 a, glm::dvec4 b, double tol);
  void IteratedExternalIntegral (double h, double error);

private:
  int m_steps_evaluation[11];

  double hproj;
  double hmin;
  double hmax;
  glm::dvec4 f_int[5];
  
  RiemannManualSummation rms;
  struct simpsonrec
  {
    double h;
    glm::dvec4 tf_b, tf_c;
    glm::dvec4 F_b, F_c;
  };

  bool m_internal_projection;
};

#endif