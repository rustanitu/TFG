/*!
\file VolumeEvaluator.h
\brief Volume Evaluation.
\author Leonardo Quatrin Campagnolo
*/

#ifndef VOLUME_EVALUATOR_H
#define VOLUME_EVALUATOR_H

#include <lqc/lqcdefines.h>

#include <lqc/Math/MGeometry.h>
#include <volrend/Volume.h>
#include <volrend/TransferFunction.h>
#include <lqc/Math/Vector3.h>
#include <lqc/Math/Vector4.h>
#include "SimpsonEvaluation.h"
#include "VolumeEvaluatorFunctions.h"

#include <cassert>

/*! Evaluates a volume in differente ways  (Riemann Summation, Simpson Adaptive...).
The availabe methods of evaluation are showed at the VolumeEvaluator's enum.
I_... is the domain the integral is calculated.
C_... is the domain of volume composition.
*/

class VolumeEvaluator
{
public: 
private:
  void* m_evaluate_function_data;

public:
  /*! Constructor*/
  VolumeEvaluator ();
  /*! Destructor*/
  ~VolumeEvaluator ();

  VolumeEvaluatorFunctions m_voleval_functions;
  bool m_use_evaluate_function;
  void SetFunctionEvaluationIndex (int index);
  void SetFunctionEvaluationData (void *data);
  void CleanFunctionEvaluationData ();

  void SetUseEvaluationFunction (bool b);

  /*! Set the volume and transfer function to be evaluated
  \param volume a lqc::Volume pointer 
  \param transfer_function a lqc::TransferFunction1D pointer.
  */
  void SetModelAndTransferFunction (vr::Volume* volume, vr::TransferFunction* transfer_function);

  /*! Set the volume and transfer function to be evaluated
  \param returncolor the final evaluated color
  \param volume the volume being used for evaluation.
  \param pixel_width the width position of the pixel evaluated
  \param pixel_height the height position of the pixel evaluated
  \param minpos the initial position of evaluation
  \param maxpos the last position of evaluation
  \return boolean checking if the evaluation was done or not.
  \bug some parameters are unused (probably added from old versions).
  */
  bool EvaluateIntegral (lqc::Vector4d *returncolor, vr::Volume *volume,
                         lqc::Vector3f minpos, lqc::Vector3f maxpos);

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
  /*! value for opacity correction when composing.
  */
  float m_opacity_correction;

  /*! Max depth recursion in adapative algorithms.
  */
  int m_adaptive_simpson_max_depth;

  /*! Evaluation types.*/
  int m_current_method;
  /*! Enum with evaluation types.*/
  enum
  {
    C_RIEMANN_R,
    C_RIEMANN_RA,
    C_RIEMANN_RGBA,
    I_RIEMANN,
    I_ADAPTIVESIMPSONPARTICIONED,
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
      n_compositions = 0;
      n_gets_tf_values = 0;
    }

    int n_gets_volume_values;
    int n_compositions;
    int n_gets_tf_values;
  };

public:
  /*! Statistics variable.
  */
  Statistics m_statistics;

private:
  /*! used to set a break point of evaluation when the composing reach the m_alpha_max_breaker value.
  */
  float m_alpha_max_breaker;
  /*! Composing domains
  */
  enum COMPOSITION_METHODS
  {
    C_R,
    C_RA,
    C_RGBA,
  };
  /*! Composing domains variable
  */
  COMPOSITION_METHODS m_composition_method;


  /*! Composite a 'stepdistance' length of the evaluation.
  \param color the returned color after Composite.
  \param value value sampled at volume tobe composed.
  \param stepdistance the length of the interval.
  */
  void Composite (lqc::Vector4d* color, float value, float stepdistance, lqc::Vector3f minpos, lqc::Vector3f evaluatedpos, bool first = false);
 
  /*! Make a Riemann Evaluation of the Volume.
  \param volume the volume being evaluated.
  \param minpos initial position of evaluation.
  \param maxpos final position of evaluation.
  \return the color of evaluation.
  */
  lqc::Vector4d Riemann (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos);
  lqc::Vector4d Riemann_I (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos);
  lqc::Vector4d AdaptiveSimpson_I (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos);

  private:
};

class EvaluationIntegral
{
public:
  EvaluationIntegral (VolumeEvaluator* veva);
  ~EvaluationIntegral ();

  static double EvaluateVolumeRenderingInnerIntergral (lqc::Vector3d p, double h, void* data);
  static lqc::Vector4d EvaluateVolumeRenderingIntergral (lqc::Vector3d p, double h, void* data);
    
  static double distanceVector3dfunc (lqc::Vector3d a, lqc::Vector3d b);
  static double distanceDoublefunc (double a, double b);
  static double colorErrorEvaluationFunc (lqc::Vector4d a, lqc::Vector4d b);

  lqc::Vector4d Evaluation (lqc::Vector3d p, double h, void* data);

  static void whenreturn (lqc::Vector4d clr, lqc::Vector3d a, lqc::Vector3d b, void* data);
  static void beforerecursion (void* data, double error);

  vr::Volume* volume;
  vr::TransferFunction* transferFunction;
  int maxdepth;
  double error;
  double ierror;
  lqc::Vector3d minpos;

  double pre_computed_opacity;

  double rec_current_opacity_eval;
  lqc::Vector3d rec_minpos;

  lqc::Vector3d m_minpos;
  lqc::Vector3d m_maxpos;
  lqc::Vector3d m_normalized_step;

  bool optimization;
  //Contador interno de iterações para avaliar a integral interna
  int inner_counter;
  //Utilizado na primeira chamada do simpson adaptativo
  // para deixar o contador setado como 1 (a-c e c-b)
  bool stay_1_inner_counter;

protected:
private:
  VolumeEvaluator* m_volumeevaluator;

  double GetOpacity (lqc::Vector4d color);
  lqc::Vector4d GetFromTransferFunction (lqc::Vector3d p);
};

#endif