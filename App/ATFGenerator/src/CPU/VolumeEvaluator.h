/*!
\file VolumeEvaluator.h
\brief Volume Evaluation.
\author Leonardo Quatrin Campagnolo
*/

#ifndef VOLUME_EVALUATOR_H
#define VOLUME_EVALUATOR_H

#include <lqc/lqcdefines.h>

#include "defines.h"

#include <math/MGeometry.h>
#include <math/Vector3.h>
#include <math/Vector4.h>
#include <volrend/Volume.h>
#include "SimpsonEvaluation.h"
#include "Evaluator.h"

#include "SimpsonHalfIntegrator.h"
#include "SimpsonErrorHalfIntegrator.h"
#include "SimpsonErrorIntegrator.h"
#include "SimpsonErrorProjectionIntegrator.h"
#include "SimpsonErrorHalfProjectionIntegrator.h"
#include "SimpsonHalfQueueIntegrator.h"
#include "SimpsonErrorQuadraticIntegrator.h"
#include "SimpsonHalfIteratorRiemannError.h"
#include "SimpsonHalfIterateIntegrator.h"

#include <cassert>

class VolumeEvaluator
{
public:
  /*! Constructor*/
  VolumeEvaluator ();
  /*! Destructor*/
  ~VolumeEvaluator ();

  /*! Set the volume and transfer function to be evaluated
  \param volume a lqc::Volume pointer 
  \param transfer_function a lqc::TransferFunction1D pointer.
  */
  void SetModelAndTransferFunction (vr::Volume* volume, vr::TransferFunction* transfer_function);

  /*! Set the volume and transfer function to be evaluated
  \param returncolor the final evaluated color
  \param volume the volume being used for evaluation.
  \param minpos the initial position of evaluation
  \param maxpos the last position of evaluation
  */
  bool EvaluateIntegral (lqc::Vector4d *returncolor, vr::Volume *volume,
                         lqc::Vector3f minpos, lqc::Vector3f maxpos, void* data = NULL);

  void BeginErrorTestStructures (int width, int height);
  bool IsCurrentErrorTest ();
  void PrintErrorTestResults ();

  bool m_alpha_truncation;

  /*! Number of samples used in adaptive simpson methods.
  */
  int m_adaptive_samples;

  double m_distance_intervals;
  double m_adaptive_distance_intervals;
  /*! the value of current error bound.
  */
  double m_integral_error;
  double m_error_inner_integral;
  /*! Number of samples used.
  */
  int m_samples;

  /*! Pointer to the current model being viewed.
  */
  vr::Volume* m_volume;
  /*! Pointer to the current transfer function being viewed.
  */
  vr::TransferFunction* m_transfer_function;

  /*! Max depth recursion in adapative algorithms.
  */
  int m_adaptive_simpson_max_depth;

  /*! Evaluation types.*/
  int m_current_method;
  bool m_normalized_interval;
  double m_min_h_step;
  double m_max_h_step;

  /*! Enum with evaluation types.*/
  enum
  {
    C_RIEMANN_SUMMATION_RIGHT_HAND,
    C_RIEMANN_SUMMATION_RIGHT_HAND_EXP_CALC,
    C_SIMPSON_RULE,

    A_SIMPSON_HALF_ITERATE,
    VOLEVAL_ADAPTIVE_SIMPSON_COUPLED,
    A_SIMPSON_HALF_ITERATE_SIMPLE_EXTSTEP,
    A_SIMPSON_HALF_ITERATE_COMPLEX_EXTSTEP,
    A_SIMPSON_HALF_ITERATE_EXP,
    A_SIMPSON_HALF_ITERATE_SEPARATED,
    VOLEVAL_ADAPTIVE_SIMPSON_DECOUPLED,

    NUMBER_OF_EVALUATIONS,

    I_ADAPTIVE_SIMPSON,
    I_ADAPTIVE_SIMPSON_PARTICIONED,

    A_SIMPSON_HALF_INTEGRATION,
    A_SIMPSON_ERROR_INTEGRATION,
    A_SIMPSON_ERROR_PROJECTION_INTEGRATION,
    A_SIMPSON_ERROR_HALF_PROJECTION_INTEGRATION,

    A_SIMPSON_HALF_QUEUE,
    A_SIMPSON_HALF_QUEUE_INTERNAL_PROJECTED,
    A_SIMPSON_HALF_QUEUE_ITERATION,

    A_SIMPSON_ERROR_HALF_INTEGRATION,
    A_SIMPSON_ERROR_QUADRATIC_INTEGRATION,

    A_SIMPSON_HALF_ITERATE_RIEMANN_ERROR,
    
    E_TESTS,
  };

  /*! Get the color (rgba) from current Transfer Function.
  \param value the amplitude (0~255) to be returned in a color value (rgba).
  \return the color (rgba) got by transfer function.
  */
  lqc::Vector4d TransferFunction (double value);
  /*! Get the respective amplitude value in volume at a received position inside the volume.
  \param volume the current volume being evaluated.
  \param pos the position being evaluated (the point MUST be inside the volume).
  \return the respective value.
  */
  float GetValueFromVolume (vr::Volume* volume, lqc::Vector3f pos);

private:
  /*! Class to compute the statistics about the evaluation.
  */
  class Statistics
  {
  public:
    Statistics ()
    {
      Reset ();
    }

    void Reset ()
    {
      n_gets_volume_values = 0;
      n_gets_tf_values = 0;
    }

    int n_gets_volume_values;
    int n_gets_tf_values;
  };

public:
  /*! Statistics variable.
  */
  Statistics m_statistics;


private:
  lqc::Vector4d C_Riemann (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos);
  lqc::Vector4d C_RiemannWithExpCalc (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos);
  lqc::Vector4d C_SimpsonRule(vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos);
  
  lqc::Vector4d I_Recursive_Adaptive_Simpson (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos);
  lqc::Vector4d I_Recursive_Adaptive_Simpson_Particioned (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos);
  
  lqc::Vector4d A_Simpson_Half_Integration (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos);
  lqc::Vector4d A_Simpson_Error_Integration (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos);
  lqc::Vector4d A_Simpson_Error_Projection_Integration (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos);
  lqc::Vector4d A_Simpson_Error_Half_Projection_Integration (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos);

  lqc::Vector4d A_Simpson_Half_Queue (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos);
  lqc::Vector4d A_Simpson_Half_Queue_Internal_Projected (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos);
  lqc::Vector4d A_Simpson_Half_Queue_Iteration (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos);

  lqc::Vector4d A_Simpson_Error_Half_Integration (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos);
  lqc::Vector4d A_Simpson_Error_Quadratic_Integration (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos);

  lqc::Vector4d A_Simpson_Half_Iterate_Riemann_Error (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos);
  lqc::Vector4d A_Simpson_Half_Iterate (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos);
  lqc::Vector4d VolEval_AdaptiveSimpsonCoupled (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos);
  lqc::Vector4d A_Simpson_Half_Iterate_Simple_ExtStep (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos);
  lqc::Vector4d A_Simpson_Half_Iterate_Complex_ExtStep (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos);
  lqc::Vector4d A_Simpson_Half_Iterate_Exp (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos);
  lqc::Vector4d A_Simpson_Half_Iterate_Separated (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos);
  lqc::Vector4d VolEval_AdaptiveSimpsonDecoupled (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos);

  lqc::Vector4d E_Tests (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos, void* data = NULL);

private:
  SimpsonHalfIntegrator a_shi;
  SimpsonErrorHalfIntegrator a_sehi;
  AdaptiveSimpsonParticionedEvaluator a_spe;
  SimpsonErrorIntegrator a_sei;
  SimpsonErrorProjectionIntegrator a_sepi;
  SimpsonErrorHalfProjectionIntegrator a_sehpi;
  SimpsonHalfQueueIntegrator a_shqi;
  SimpsonErrorQuadraticIntegrator a_seqi;
  SimpsonHalfIteratorRiemannError a_shire;
  SimpsonHalfIterateIntegrator a_shii;
};

#endif