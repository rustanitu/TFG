#include "VolumeEvaluator.h"

#include <volrend/Volume.h>
#include <volrend/TransferFunction.h>

#include "RiemannSummation.hpp"

VolumeEvaluator::VolumeEvaluator ()
  : m_volume (NULL),
  m_alpha_truncation (false),
  m_normalized_interval (false),
  a_shi (this),
  a_sehi (this),
  a_sei (this),
  a_sepi (this),
  a_sehpi (this),
  a_shqi (this),
  a_seqi (this),
  a_shire (this),
  a_shii (this)
{
  m_current_method = C_RIEMANN_SUMMATION_RIGHT_HAND;
  m_transfer_function = NULL;
  m_adaptive_simpson_max_depth = 100;
  m_integral_error = 1.0;
  m_error_inner_integral = 1.0;
  m_adaptive_distance_intervals = 32.0;
  m_distance_intervals = 128.0;
  m_min_h_step = 0.4;
  m_max_h_step = 4.0;
}
 
VolumeEvaluator::~VolumeEvaluator ()
{}

void VolumeEvaluator::SetModelAndTransferFunction(vr::Volume* ivolume, vr::TransferFunction* itransfer_function)
{
  m_volume = ivolume;
  m_transfer_function = itransfer_function;
}

bool VolumeEvaluator::EvaluateIntegral(lqc::Vector4d *returncolor, vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos, void* data)
{
  (*returncolor) = lqc::Vector4d (-1.f);
  if (!volume) return false;
  if (!m_transfer_function) return false;

  switch (m_current_method)
  {
    case C_RIEMANN_SUMMATION_RIGHT_HAND:
      (*returncolor) = C_Riemann(volume, minpos, maxpos);
      break;
    case C_RIEMANN_SUMMATION_RIGHT_HAND_EXP_CALC:
      (*returncolor) = C_RiemannWithExpCalc(volume, minpos, maxpos);
      break;
    case C_SIMPSON_RULE:
      (*returncolor) = C_SimpsonRule(volume, minpos, maxpos);
      break;

    case I_ADAPTIVE_SIMPSON:
      (*returncolor) = I_Recursive_Adaptive_Simpson(volume, minpos, maxpos);
      break;
    case I_ADAPTIVE_SIMPSON_PARTICIONED:
      (*returncolor) = I_Recursive_Adaptive_Simpson_Particioned(volume, minpos, maxpos);
      break;
    
    case A_SIMPSON_HALF_INTEGRATION:
      (*returncolor) = A_Simpson_Half_Integration(volume, minpos, maxpos);
      break;
    case A_SIMPSON_ERROR_INTEGRATION:
      (*returncolor) = A_Simpson_Error_Integration(volume, minpos, maxpos);
      break;
    case A_SIMPSON_ERROR_PROJECTION_INTEGRATION:
      (*returncolor) = A_Simpson_Error_Projection_Integration(volume, minpos, maxpos);
      break;
    case A_SIMPSON_ERROR_HALF_PROJECTION_INTEGRATION:
      (*returncolor) = A_Simpson_Error_Half_Projection_Integration(volume, minpos, maxpos);
      break;

    case A_SIMPSON_HALF_QUEUE:
      (*returncolor) = A_Simpson_Half_Queue(volume, minpos, maxpos);
      break;
    case A_SIMPSON_HALF_QUEUE_INTERNAL_PROJECTED:
      (*returncolor) = A_Simpson_Half_Queue_Internal_Projected(volume, minpos, maxpos);
      break;
    case A_SIMPSON_HALF_QUEUE_ITERATION:
      (*returncolor) = A_Simpson_Half_Queue_Iteration(volume, minpos, maxpos);
      break;

    case A_SIMPSON_ERROR_HALF_INTEGRATION:
      (*returncolor) = A_Simpson_Error_Half_Integration(volume, minpos, maxpos);
      break;
    case A_SIMPSON_ERROR_QUADRATIC_INTEGRATION:
      (*returncolor) = A_Simpson_Error_Quadratic_Integration(volume, minpos, maxpos);
      break;

    case A_SIMPSON_HALF_ITERATE_RIEMANN_ERROR:
      (*returncolor) = A_Simpson_Half_Iterate_Riemann_Error(volume, minpos, maxpos);
      break;
    case A_SIMPSON_HALF_ITERATE:
      (*returncolor) = A_Simpson_Half_Iterate(volume, minpos, maxpos);
      break;
    case VOLEVAL_ADAPTIVE_SIMPSON_COUPLED:
      (*returncolor) = VolEval_AdaptiveSimpsonCoupled(volume, minpos, maxpos);
      break;
    case A_SIMPSON_HALF_ITERATE_SIMPLE_EXTSTEP:
      (*returncolor) = A_Simpson_Half_Iterate_Simple_ExtStep(volume, minpos, maxpos);
      break;
    case A_SIMPSON_HALF_ITERATE_COMPLEX_EXTSTEP:
      (*returncolor) = A_Simpson_Half_Iterate_Complex_ExtStep(volume, minpos, maxpos);
      break;
    case A_SIMPSON_HALF_ITERATE_EXP:
      (*returncolor) = A_Simpson_Half_Iterate_Exp(volume, minpos, maxpos);
      break;
    case A_SIMPSON_HALF_ITERATE_SEPARATED:
      (*returncolor) = A_Simpson_Half_Iterate_Separated(volume, minpos, maxpos);
      break;
    case VOLEVAL_ADAPTIVE_SIMPSON_DECOUPLED:
      (*returncolor) = VolEval_AdaptiveSimpsonDecoupled(volume, minpos, maxpos);
      break;
    case E_TESTS:
      (*returncolor) = E_Tests(volume, minpos, maxpos, data);
      break;
  }
  return true;
}

void VolumeEvaluator::BeginErrorTestStructures (int width, int height)
{
  m_statistics.Reset ();
  switch (m_current_method)
  {
  case A_SIMPSON_HALF_INTEGRATION:
#ifdef ANALYSIS__STORE_INTEGRATE_INTERVAL_TIMES
      SimpsonHalfIntegrator::s_IntegrativeInternalTimes = 0;
      SimpsonHalfIntegrator::s_IntegrativeExternalTimes = 0;

      SimpsonHalfIntegrator::s_RecalculateIntegrativeExternalTimes = 0;
      SimpsonHalfIntegrator::s_RecalculateIntegrativeInternalTimes = 0;
#endif
      break;
    case A_SIMPSON_ERROR_HALF_INTEGRATION:
#ifdef ANALYSIS__STORE_INTEGRATE_INTERVAL_TIMES
      SimpsonErrorHalfIntegrator::s_IntegrativeInternalTimes = 0;
      SimpsonErrorHalfIntegrator::s_IntegrativeExternalTimes = 0;

      SimpsonErrorHalfIntegrator::s_RecalculateIntegrativeExternalTimes = 0;
      SimpsonErrorHalfIntegrator::s_RecalculateIntegrativeInternalTimes = 0;
#endif
      break;
    case A_SIMPSON_ERROR_INTEGRATION:
#ifdef ANALYSIS__STORE_INTEGRATE_INTERVAL_TIMES
      SimpsonErrorIntegrator::s_IntegrativeInternalTimes = 0;
      SimpsonErrorIntegrator::s_IntegrativeExternalTimes = 0;

      SimpsonErrorIntegrator::s_RecalculateIntegrativeExternalTimes = 0;
      SimpsonErrorIntegrator::s_RecalculateIntegrativeInternalTimes = 0;
#endif
      a_sei.Reset ();
      break;
    case A_SIMPSON_ERROR_QUADRATIC_INTEGRATION:
#ifdef ANALYSIS__STORE_INTEGRATE_INTERVAL_TIMES
      SimpsonErrorQuadraticIntegrator::s_IntegrativeInternalTimes = 0;
      SimpsonErrorQuadraticIntegrator::s_IntegrativeExternalTimes = 0;

      SimpsonErrorQuadraticIntegrator::s_RecalculateIntegrativeExternalTimes = 0;
      SimpsonErrorQuadraticIntegrator::s_RecalculateIntegrativeInternalTimes = 0;
#endif
      a_seqi.Reset ();
      break;
    case A_SIMPSON_HALF_QUEUE_INTERNAL_PROJECTED:
    case A_SIMPSON_HALF_QUEUE_ITERATION:
    case A_SIMPSON_HALF_QUEUE:
      a_shqi.Reset ();
      break;
    case A_SIMPSON_HALF_ITERATE_RIEMANN_ERROR:
      a_shire.Reset ();
      break;
    case A_SIMPSON_HALF_ITERATE:
    case VOLEVAL_ADAPTIVE_SIMPSON_COUPLED:
    case A_SIMPSON_HALF_ITERATE_SIMPLE_EXTSTEP:
    case A_SIMPSON_HALF_ITERATE_COMPLEX_EXTSTEP:
    case A_SIMPSON_HALF_ITERATE_EXP:
    case A_SIMPSON_HALF_ITERATE_SEPARATED:
    case VOLEVAL_ADAPTIVE_SIMPSON_DECOUPLED:
      a_shii.Reset ();
      break;
    default:
      break;
  }
}

bool VolumeEvaluator::IsCurrentErrorTest ()
{
  switch (m_current_method)
  {
    case C_RIEMANN_SUMMATION_RIGHT_HAND:
    case I_ADAPTIVE_SIMPSON_PARTICIONED:
    case A_SIMPSON_HALF_INTEGRATION:
    case A_SIMPSON_ERROR_INTEGRATION:
    case A_SIMPSON_ERROR_PROJECTION_INTEGRATION:
    case A_SIMPSON_ERROR_HALF_PROJECTION_INTEGRATION:
    case A_SIMPSON_HALF_QUEUE:
    case A_SIMPSON_HALF_QUEUE_INTERNAL_PROJECTED:
    case A_SIMPSON_HALF_QUEUE_ITERATION:
    case A_SIMPSON_ERROR_HALF_INTEGRATION:
    case A_SIMPSON_ERROR_QUADRATIC_INTEGRATION:
    case A_SIMPSON_HALF_ITERATE_RIEMANN_ERROR:
    case A_SIMPSON_HALF_ITERATE:
    case VOLEVAL_ADAPTIVE_SIMPSON_COUPLED:
    case A_SIMPSON_HALF_ITERATE_SIMPLE_EXTSTEP:
    case A_SIMPSON_HALF_ITERATE_COMPLEX_EXTSTEP:
    case A_SIMPSON_HALF_ITERATE_EXP:
    case A_SIMPSON_HALF_ITERATE_SEPARATED:
    case VOLEVAL_ADAPTIVE_SIMPSON_DECOUPLED:
      return true;
      break;
    default:
      break;

  }
  return false;
}

void VolumeEvaluator::PrintErrorTestResults ()
{
  switch (m_current_method)
  {
  case A_SIMPSON_HALF_INTEGRATION:
      printf ("Resultados: Simpson Half\n");
#ifdef ANALYSIS__STORE_INTEGRATE_INTERVAL_TIMES
      printf ("  Avaliacoes feitas da integral:\n");
      printf ("    Externa: %d\n", SimpsonHalfIntegrator::s_IntegrativeExternalTimes);
      printf ("    Interna: %d\n", SimpsonHalfIntegrator::s_IntegrativeInternalTimes);

      printf ("  Recalculos de intervalos feitos:\n");
      printf ("    Externa: %d\n", SimpsonHalfIntegrator::s_RecalculateIntegrativeExternalTimes);
      printf ("    Interna: %d\n", SimpsonHalfIntegrator::s_RecalculateIntegrativeInternalTimes);
#endif
      break;
    case A_SIMPSON_ERROR_INTEGRATION:
      printf ("Resultados: Simpson Error\n");
#ifdef ANALYSIS__STORE_INTEGRATE_INTERVAL_TIMES
      printf ("  Avaliacoes feitas da integral:\n");
      printf ("    Externa: %d\n", SimpsonErrorIntegrator::s_IntegrativeExternalTimes);
      printf ("    Interna: %d\n", SimpsonErrorIntegrator::s_IntegrativeInternalTimes);

      printf ("  Recalculos de intervalos feitos:\n");
      printf ("    Externa: %d\n", SimpsonErrorIntegrator::s_RecalculateIntegrativeExternalTimes);
      printf ("    Interna: %d\n", SimpsonErrorIntegrator::s_RecalculateIntegrativeInternalTimes);
#endif
      printf ("  Erro extrapolado por projecoes:\n");
      printf ("    Projecao Interna:\n      %lf\n", a_sei.m_clc_proj_newinternalprojection);
      printf ("    Resultado Interna:\n      %lf\n", a_sei.m_clc_proj_internal_error);
      printf ("    Resultado Externa:\n      %lf\n      %lf\n      %lf\n      %lf\n", 
        a_sei.m_clc_proj_external_error.x, a_sei.m_clc_proj_external_error.y,
        a_sei.m_clc_proj_external_error.z, a_sei.m_clc_proj_external_error.w);

      break;
    case A_SIMPSON_ERROR_PROJECTION_INTEGRATION:
      printf ("Resultados: Simpson Error Projection\n");
      break;
    case A_SIMPSON_ERROR_HALF_PROJECTION_INTEGRATION:
      printf ("Resultados: Simpson Error Half Projection\n");
      break;
    case A_SIMPSON_HALF_QUEUE:
    case A_SIMPSON_HALF_QUEUE_INTERNAL_PROJECTED:
      printf ("Resultados: Simpson I - [Error Half Projection] E - [Half]\n");
      break;
    case A_SIMPSON_ERROR_HALF_INTEGRATION:
      printf ("Resultados: Simpson Error Half\n");
#ifdef ANALYSIS__STORE_INTEGRATE_INTERVAL_TIMES
      printf ("  Avaliacoes feitas da integral:\n");
      printf ("    Externa: %d\n", SimpsonErrorHalfIntegrator::s_IntegrativeExternalTimes);
      printf ("    Interna: %d\n", SimpsonErrorHalfIntegrator::s_IntegrativeInternalTimes);

      printf ("  Recalculos de intervalos feitos:\n");
      printf ("    Externa: %d\n", SimpsonErrorHalfIntegrator::s_RecalculateIntegrativeExternalTimes);
      printf ("    Interna: %d\n", SimpsonErrorHalfIntegrator::s_RecalculateIntegrativeInternalTimes);
#endif
      break;
    case A_SIMPSON_ERROR_QUADRATIC_INTEGRATION:
      printf ("Resultados: Simpson Error Quadratic\n");
#ifdef ANALYSIS__STORE_INTEGRATE_INTERVAL_TIMES
      printf ("  Avaliacoes feitas da integral:\n");
      printf ("    Externa: %d\n", AdaptiveSimpsonErrorQuadraticIntegrator::s_IntegrativeExternalTimes);
      printf ("    Interna: %d\n", AdaptiveSimpsonErrorQuadraticIntegrator::s_IntegrativeInternalTimes);

      printf ("  Recalculos de intervalos feitos:\n");
      printf ("    Externa: %d\n", AdaptiveSimpsonErrorQuadraticIntegrator::s_RecalculateIntegrativeExternalTimes);
      printf ("    Interna: %d\n", AdaptiveSimpsonErrorQuadraticIntegrator::s_RecalculateIntegrativeInternalTimes);
#endif
      break;
    case A_SIMPSON_HALF_QUEUE_ITERATION:
      printf ("Resultados: Simpson Half Iteration\n");
      a_shqi.PrintStepsEvaluation ();
      break;
    case A_SIMPSON_HALF_ITERATE_RIEMANN_ERROR:
      printf ("Resultados: Simpson Half Iteration with S2 = Riemann 0.01\n");
      a_shire.PrintStepsEvaluation ();
      break;
    case A_SIMPSON_HALF_ITERATE:
    case VOLEVAL_ADAPTIVE_SIMPSON_COUPLED:
    case A_SIMPSON_HALF_ITERATE_SIMPLE_EXTSTEP:
    case A_SIMPSON_HALF_ITERATE_COMPLEX_EXTSTEP:
    case A_SIMPSON_HALF_ITERATE_EXP:
    case A_SIMPSON_HALF_ITERATE_SEPARATED:
    case VOLEVAL_ADAPTIVE_SIMPSON_DECOUPLED:
      printf ("Resultados: Simpson Half Iterate\n");
      a_shii.PrintStepsEvaluation ();
      break;
    default:
      printf ("The Current Method is not a test\n");
      break;
  }

#ifdef STATISTICS_GETS
  printf ("Estatisticas:\n");
  printf ("  - Acessos no dado: %d\n", m_statistics.n_gets_volume_values);
  printf ("  - Acessos na ft: %d\n", m_statistics.n_gets_tf_values);
#endif
}

lqc::Vector4d VolumeEvaluator::TransferFunction (double value)
{
#ifdef STATISTICS_GETS
  m_statistics.n_gets_tf_values++;
#endif
  return m_transfer_function->Get (value);
}

float VolumeEvaluator::GetValueFromVolume (vr::Volume* volume, lqc::Vector3f pos)
{
#ifdef STATISTICS_GETS
  m_statistics.n_gets_volume_values++;
#endif
  return volume->InterpolatedValue (pos);
}

lqc::Vector4d VolumeEvaluator::C_Riemann (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos)
{
  lqc::Vector4d color (0);
  lqc::Vector3d step = lqc::Vector3d::Normalize (lqc::Vector3d (maxpos.x, maxpos.y, maxpos.z) - lqc::Vector3d (minpos.x, minpos.y, minpos.z));
  lqc::Vector3d dminpos = lqc::Vector3d (minpos.x, minpos.y, minpos.z);

  double s1 = lqc::Distance (minpos, maxpos);
  double s0 = 0;
  double h = m_distance_intervals;
  double s = s0;
  RiemannSummation rs (this, m_volume, m_transfer_function);

  while (s + h < s1)
  {
    lqc::Vector3d p = dminpos + (s + h)*step;
    rs.Composite (&color, p, h);
    s = s + h;
  }
  lqc::Vector3d p = dminpos + (s1)*step;
  rs.Composite (&color, p, s1 - s);

  return color;
}

lqc::Vector4d VolumeEvaluator::C_RiemannWithExpCalc (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos)
{
  lqc::Vector4d color (0);
  lqc::Vector3d step = lqc::Vector3d::Normalize (lqc::Vector3d (maxpos.x, maxpos.y, maxpos.z) - lqc::Vector3d (minpos.x, minpos.y, minpos.z));
  lqc::Vector3d dminpos = lqc::Vector3d (minpos.x, minpos.y, minpos.z);

  double s1 = lqc::Distance (minpos, maxpos);
  double s0 = 0;
  double h = m_distance_intervals;
  double s = s0;

  RiemannExpManualSummation res (this);
  res.Init (m_volume, m_transfer_function, dminpos, step);

  while (s < s1)
  {
    h = MIN (h, s1 - s);

    lqc::Vector3d p = dminpos + (s + h)*step;
    res.SumAndComposite (&color, p, h);

    s = s + h;
  }
  return color;
}

lqc::Vector4d VolumeEvaluator::C_SimpsonRule(vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos)
{
  lqc::Vector4d color (0);
  lqc::Vector3d step = lqc::Vector3d::Normalize (lqc::Vector3d (maxpos.x, maxpos.y, maxpos.z) - lqc::Vector3d (minpos.x, minpos.y, minpos.z));
  lqc::Vector3d dminpos = lqc::Vector3d (minpos.x, minpos.y, minpos.z);

  double s1 = lqc::Distance (minpos, maxpos);
  double s0 = 0;
  double h = m_distance_intervals;
  double s = s0;

  double exp_opacity = 1.0;
  double opacity = 0.0;
  double a_internal, h_12;

  lqc::Vector3d p2, p1;
  lqc::Vector3d p3 = dminpos;

  lqc::Vector4d tf_a, tf_c, tf_d, tf_e, F_a, F_c, F_b;
  lqc::Vector4d tf_b = TransferFunction (GetValueFromVolume (volume, lqc::Vector3f (p3.x, p3.y, p3.z)));

  while (s < s1)
  {
    h = MIN (h, s1 - s);

    p1 = p3;
    p2 = dminpos + (s + h * 0.5) * step;
    p3 = dminpos + (s + h) * step;
    h_12 = h / 12.0;

    tf_a = tf_b;
    tf_c = TransferFunction (GetValueFromVolume (volume, lqc::Vector3f (p2.x, p2.y, p2.z)));
    tf_b = TransferFunction (GetValueFromVolume (volume, lqc::Vector3f (p3.x, p3.y, p3.z)));
    tf_d = TransferFunction (GetValueFromVolume (volume, lqc::Vector3f ((p1.x + p2.x) * 0.5, (p1.y + p2.y) * 0.5, (p1.z + p2.z) * 0.5)));
    tf_e = TransferFunction (GetValueFromVolume (volume, lqc::Vector3f ((p3.x + p2.x) * 0.5, (p3.y + p2.y) * 0.5, (p3.z + p2.z) * 0.5)));

    a_internal = tf_a.w * exp_opacity;
    F_a = lqc::Vector4d (tf_a.x*a_internal, tf_a.y*a_internal, tf_a.z*a_internal, a_internal);

    opacity += h_12*(tf_a.w + 4.0 * tf_d.w + tf_c.w);
    a_internal = tf_c.w * std::exp (-opacity);
    F_c = lqc::Vector4d (tf_c.x*a_internal, tf_c.y*a_internal, tf_c.z*a_internal, a_internal);

    opacity += h_12*(tf_c.w + 4.0 * tf_e.w + tf_b.w);
    exp_opacity = std::exp (-opacity);
    a_internal = tf_b.w * exp_opacity;
    F_b = lqc::Vector4d (tf_b.x*a_internal, tf_b.y*a_internal, tf_b.z*a_internal, a_internal);

    color += (h_12 * 2.0) * (F_a + 4.0 * F_c + F_b);

    s = s + h;
  }
  return color;
}

lqc::Vector4d VolumeEvaluator::I_Recursive_Adaptive_Simpson (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos)
{
  a_spe.SetParameters (AdaptiveSimpsonParticionedEvaluator::Parameters (
    m_integral_error, m_error_inner_integral, m_adaptive_distance_intervals,
    m_adaptive_simpson_max_depth, volume, m_transfer_function, this));

  a_spe.Init (lqc::Vector3d (minpos.x, minpos.y, minpos.z),
    lqc::Vector3d (maxpos.x, maxpos.y, maxpos.z));

  a_spe.RunEvaluation (0, lqc::Distance (minpos, maxpos));

  AdaptiveSimpsonParticionedEvaluator::Result result;
  a_spe.End (&result);

  return result.color;
}

lqc::Vector4d VolumeEvaluator::I_Recursive_Adaptive_Simpson_Particioned (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos)
{
  a_spe.SetParameters (AdaptiveSimpsonParticionedEvaluator::Parameters (
    m_integral_error, m_error_inner_integral, m_adaptive_distance_intervals, 
    m_adaptive_simpson_max_depth, volume, m_transfer_function, this));

  a_spe.Init (lqc::Vector3d (minpos.x, minpos.y, minpos.z),
    lqc::Vector3d (maxpos.x, maxpos.y, maxpos.z));
  
  a_spe.RunEvaluation (0, lqc::Distance (minpos, maxpos));
  
  AdaptiveSimpsonParticionedEvaluator::Result result;
  a_spe.End (&result);

  return result.color;
}

lqc::Vector4d VolumeEvaluator::A_Simpson_Half_Integration (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos)
{
  a_shi.Init (lqc::Vector3d (minpos.x, minpos.y, minpos.z),
    lqc::Vector3d (maxpos.x, maxpos.y, maxpos.z), m_volume, m_transfer_function);
  double s1 = lqc::Distance (minpos, maxpos);
  a_shi.Integrate (0, s1, m_integral_error, INTEGRATOR_INITIAL_STEP);
  return a_shi.color;
}

lqc::Vector4d VolumeEvaluator::A_Simpson_Error_Integration (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos)
{
  a_sei.Init (lqc::Vector3d (minpos.x, minpos.y, minpos.z), lqc::Vector3d (maxpos.x, maxpos.y, maxpos.z), m_volume, m_transfer_function);
  double s1 = lqc::Distance (minpos, maxpos);
  a_sei.Integrate (0, s1, m_integral_error, INTEGRATOR_INITIAL_STEP);
  return a_sei.color;
}

lqc::Vector4d VolumeEvaluator::A_Simpson_Error_Projection_Integration (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos)
{
  a_sepi.Init (lqc::Vector3d (minpos.x, minpos.y, minpos.z), lqc::Vector3d (maxpos.x, maxpos.y, maxpos.z), m_volume, m_transfer_function);
  double s1 = lqc::Distance (minpos, maxpos);
  a_sepi.Integrate (0, s1, m_integral_error, INTEGRATOR_INITIAL_STEP);
  return a_sepi.color;
}

lqc::Vector4d VolumeEvaluator::A_Simpson_Error_Half_Projection_Integration (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos)
{
  a_sehpi.Init (lqc::Vector3d (minpos.x, minpos.y, minpos.z), lqc::Vector3d (maxpos.x, maxpos.y, maxpos.z), m_volume, m_transfer_function);
  double s1 = lqc::Distance (minpos, maxpos);
  a_sehpi.Integrate (0, s1, m_integral_error, INTEGRATOR_INITIAL_STEP);
  return a_sehpi.color;
}

lqc::Vector4d VolumeEvaluator::A_Simpson_Half_Queue (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos)
{
  a_shqi.Init (lqc::Vector3d (minpos.x, minpos.y, minpos.z), lqc::Vector3d (maxpos.x, maxpos.y, maxpos.z), m_volume, m_transfer_function);
  double s1 = lqc::Distance (minpos, maxpos);
  a_shqi.Integrate (0, s1, m_integral_error, INTEGRATOR_INITIAL_STEP, m_min_h_step, m_max_h_step);
  return a_shqi.color;
}

lqc::Vector4d VolumeEvaluator::A_Simpson_Half_Queue_Internal_Projected (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos)
{
  a_shqi.Init (lqc::Vector3d (minpos.x, minpos.y, minpos.z), lqc::Vector3d (maxpos.x, maxpos.y, maxpos.z), m_volume, m_transfer_function);
  double s1 = lqc::Distance (minpos, maxpos);
  a_shqi.IntegrateInternalProjected (0, s1, m_integral_error, INTEGRATOR_INITIAL_STEP, m_min_h_step, m_max_h_step);
  
  return a_shqi.color;
}

lqc::Vector4d VolumeEvaluator::A_Simpson_Half_Queue_Iteration (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos)
{
  a_shqi.Init (lqc::Vector3d (minpos.x, minpos.y, minpos.z), lqc::Vector3d (maxpos.x, maxpos.y, maxpos.z), m_volume, m_transfer_function);
  double s1 = lqc::Distance (minpos, maxpos);
  a_shqi.IntegrateIterated (0, s1, m_integral_error, INTEGRATOR_INITIAL_STEP, m_min_h_step, m_max_h_step);

  return a_shqi.color;
}

lqc::Vector4d VolumeEvaluator::A_Simpson_Error_Half_Integration (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos)
{
  a_sehi.Init (lqc::Vector3d (minpos.x, minpos.y, minpos.z), lqc::Vector3d (maxpos.x, maxpos.y, maxpos.z), m_volume, m_transfer_function);
  double s1 = lqc::Distance (minpos, maxpos);
  a_sehi.Integrate (0, s1, m_integral_error, INTEGRATOR_INITIAL_STEP);
  return a_sehi.color;
}

lqc::Vector4d VolumeEvaluator::A_Simpson_Error_Quadratic_Integration (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos)
{
  a_seqi.Init (lqc::Vector3d (minpos.x, minpos.y, minpos.z), lqc::Vector3d (maxpos.x, maxpos.y, maxpos.z), m_volume, m_transfer_function);
  double s1 = lqc::Distance (minpos, maxpos);
  a_seqi.Integrate (0, s1, m_integral_error, INTEGRATOR_INITIAL_STEP);
  return a_seqi.color;
}

lqc::Vector4d VolumeEvaluator::A_Simpson_Half_Iterate_Riemann_Error (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos)
{
  a_shire.Init (lqc::Vector3d (minpos.x, minpos.y, minpos.z), lqc::Vector3d (maxpos.x, maxpos.y, maxpos.z), m_volume, m_transfer_function);
  double s1 = lqc::Distance (minpos, maxpos);
  a_shire.Integrate (0, s1, m_integral_error, INTEGRATOR_INITIAL_STEP, m_min_h_step, m_max_h_step);
  return a_shire.color;
}

lqc::Vector4d VolumeEvaluator::A_Simpson_Half_Iterate (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos)
{
  a_shii.Init (lqc::Vector3d (minpos.x, minpos.y, minpos.z), lqc::Vector3d (maxpos.x, maxpos.y, maxpos.z), m_volume, m_transfer_function);
  double s1 = lqc::Distance (minpos, maxpos);
  a_shii.CoupledIntegration (0, s1, m_integral_error, INTEGRATOR_INITIAL_STEP, m_min_h_step, m_max_h_step);
  return a_shii.color;
}

lqc::Vector4d VolumeEvaluator::VolEval_AdaptiveSimpsonCoupled (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos)
{
  SimpsonHalfIterateIntegrator shit (this);
  shit.Init (lqc::Vector3d (minpos.x, minpos.y, minpos.z), lqc::Vector3d (maxpos.x, maxpos.y, maxpos.z), m_volume, m_transfer_function);
  double s1 = lqc::Distance (minpos, maxpos);
  shit.CoupledIntegration (0, s1, m_integral_error, INTEGRATOR_INITIAL_STEP, m_min_h_step, m_max_h_step);
  return shit.color;
}

lqc::Vector4d VolumeEvaluator::A_Simpson_Half_Iterate_Simple_ExtStep (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos)
{
  a_shii.Init (lqc::Vector3d (minpos.x, minpos.y, minpos.z), lqc::Vector3d (maxpos.x, maxpos.y, maxpos.z), m_volume, m_transfer_function);
  double s1 = lqc::Distance (minpos, maxpos);
  a_shii.IntegrateScount (0, s1, m_integral_error, INTEGRATOR_INITIAL_STEP, m_min_h_step, m_max_h_step);

  return a_shii.color;
}

lqc::Vector4d VolumeEvaluator::A_Simpson_Half_Iterate_Complex_ExtStep (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos)
{
  a_shii.Init (lqc::Vector3d (minpos.x, minpos.y, minpos.z), lqc::Vector3d (maxpos.x, maxpos.y, maxpos.z), m_volume, m_transfer_function);
  double s1 = lqc::Distance (minpos, maxpos);
  a_shii.IntegrateComplexExtStep (0, s1, m_integral_error, INTEGRATOR_INITIAL_STEP, m_min_h_step, m_max_h_step);

  return a_shii.color;
}

lqc::Vector4d VolumeEvaluator::A_Simpson_Half_Iterate_Exp (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos)
{
  a_shii.Init (lqc::Vector3d (minpos.x, minpos.y, minpos.z), lqc::Vector3d (maxpos.x, maxpos.y, maxpos.z), m_volume, m_transfer_function);
  double s1 = lqc::Distance (minpos, maxpos);
  a_shii.IntegrateExp (0, s1, m_integral_error, INTEGRATOR_INITIAL_STEP, m_min_h_step, m_max_h_step);

  return a_shii.color;
}

lqc::Vector4d VolumeEvaluator::A_Simpson_Half_Iterate_Separated (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos)
{
  a_shii.Init (lqc::Vector3d (minpos.x, minpos.y, minpos.z), lqc::Vector3d (maxpos.x, maxpos.y, maxpos.z), m_volume, m_transfer_function);
  double s1 = lqc::Distance (minpos, maxpos);
  a_shii.IntegrateSeparated (0, s1, m_integral_error, INTEGRATOR_INITIAL_STEP, m_min_h_step, m_max_h_step);

  return a_shii.color;
}

lqc::Vector4d VolumeEvaluator::VolEval_AdaptiveSimpsonDecoupled (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos)
{
  a_shii.Init (lqc::Vector3d (minpos.x, minpos.y, minpos.z), lqc::Vector3d (maxpos.x, maxpos.y, maxpos.z), m_volume, m_transfer_function);
  double s1 = lqc::Distance (minpos, maxpos);
  a_shii.DecoupledIntegration (0, s1, m_integral_error, INTEGRATOR_INITIAL_STEP, m_min_h_step, m_max_h_step);

  return a_shii.color;
}

lqc::Vector4d VolumeEvaluator::E_Tests (vr::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos, void* data)
{
  lqc::Vector3d step = lqc::Vector3d::Normalize (lqc::Vector3d (maxpos.x, maxpos.y, maxpos.z) - lqc::Vector3d (minpos.x, minpos.y, minpos.z));
  lqc::Vector3d dminpos = lqc::Vector3d (minpos.x, minpos.y, minpos.z);

  double s1 = lqc::Distance (minpos, maxpos);
  double s0 = 0;
  double h;
  double s;

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  {
    RiemannSummation rs (this, m_volume, m_transfer_function);
#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
    std::ofstream rgba_rfile;
    rgba_rfile.open ("Riemann Summation h_0_01.txt");
    rs.file = &rgba_rfile;

    rgba_rfile << std::setprecision (30) << 0 << '\t'
      << std::setprecision (30) << 0 << '\t'
      << std::setprecision (30) << 0 << '\t'
      << std::setprecision (30) << 0 << '\t'
      << std::setprecision (30) << 0 << '\t'
      << std::setprecision (30) << 0 << '\n';
#endif
    h = 0.01;
    s = s0;
    lqc::Vector4d rs_0_01_color (0);

    while (s < s1)
    {
      h = MIN (h, s1 - s);

      lqc::Vector3d p = dminpos + (s + h)*step;
      rs.Composite (&rs_0_01_color, p, h);
      s = s + h;
    }

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
    rgba_rfile.close ();
#endif
  }
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  {
    RiemannSummation rs (this, m_volume, m_transfer_function);
#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
    std::ofstream rgba_rfile;
    rgba_rfile.open ("Riemann Summation h_1.txt");
    rs.file = &rgba_rfile;

    rgba_rfile << std::setprecision (30) << 0 << '\t'
      << std::setprecision (30) << 0 << '\t'
      << std::setprecision (30) << 0 << '\t'
      << std::setprecision (30) << 0 << '\t'
      << std::setprecision (30) << 0 << '\t'
      << std::setprecision (30) << 0 << '\n';
#endif
    h = 1.0;
    s = s0;
    lqc::Vector4d rs_1_color (0);

    while (s < s1)
    {
      h = MIN (h, s1 - s);

      lqc::Vector3d p = dminpos + (s + h)*step;
      rs.Composite (&rs_1_color, p, h);
      s = s + h;
    }

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
    rgba_rfile.close ();
#endif
  }
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  {
    RiemannSummation rs (this, m_volume, m_transfer_function);
#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
    std::ofstream rgba_rfile;
    rgba_rfile.open ("Riemann Summation h_0_5.txt");
    rs.file = &rgba_rfile;

    rgba_rfile << std::setprecision (30) << 0 << '\t'
      << std::setprecision (30) << 0 << '\t'
      << std::setprecision (30) << 0 << '\t'
      << std::setprecision (30) << 0 << '\t'
      << std::setprecision (30) << 0 << '\t'
      << std::setprecision (30) << 0 << '\n';
#endif
    h = 0.5;
    s = s0;
    lqc::Vector4d rs_0_5_color (0);
    while (s < s1)
    {
      h = MIN (h, s1 - s);

      lqc::Vector3d p = dminpos + (s + h)*step;
      rs.Composite (&rs_0_5_color, p, h);
      s = s + h;
    }

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
    rgba_rfile.close ();
#endif
  }
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  {
    a_shii.Init (lqc::Vector3d (minpos.x, minpos.y, minpos.z), lqc::Vector3d (maxpos.x, maxpos.y, maxpos.z), m_volume, m_transfer_function);
#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
    std::ofstream rgba_rfile_ext;
    rgba_rfile_ext.open ("Coupled Adaptive 0_01 EXT.txt");
    a_shii.file_ext = &rgba_rfile_ext;

    rgba_rfile_ext << std::setprecision (30) << 0 << '\t'
      << std::setprecision (30) << 0 << '\t'
      << std::setprecision (30) << 0 << '\t'
      << std::setprecision (30) << 0 << '\t'
      << std::setprecision (30) << 0 << '\t'
      << std::setprecision (30) << 0 << '\n';

    std::ofstream rgba_rfile_int;
    rgba_rfile_int.open ("Coupled Adaptive 0_01 INT.txt");
    a_shii.file_int = &rgba_rfile_int;

    rgba_rfile_int << std::setprecision (30) << 0 << '\t'
      << std::setprecision (30) << 0 << '\t'
      << std::setprecision (30) << 0 << '\t'
      << std::setprecision (30) << 0 << '\t'
      << std::setprecision (30) << 0 << '\t'
      << std::setprecision (30) << 0 << '\n';
#endif
    double s1 = lqc::Distance (minpos, maxpos);
    a_shii.CoupledIntegration (0, s1, 0.01, INTEGRATOR_INITIAL_STEP, m_min_h_step, m_max_h_step);
#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
    rgba_rfile_ext.close ();
    rgba_rfile_int.close ();
#endif
  }
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
    {
      a_shii.Init (lqc::Vector3d (minpos.x, minpos.y, minpos.z), lqc::Vector3d (maxpos.x, maxpos.y, maxpos.z), m_volume, m_transfer_function);
#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
      std::ofstream rgba_rfile_ext;
      rgba_rfile_ext.open ("Coupled Adaptive 0_001 EXT.txt");
      a_shii.file_ext = &rgba_rfile_ext;

      rgba_rfile_ext << std::setprecision (30) << 0 << '\t'
        << std::setprecision (30) << 0 << '\t'
        << std::setprecision (30) << 0 << '\t'
        << std::setprecision (30) << 0 << '\t'
        << std::setprecision (30) << 0 << '\t'
        << std::setprecision (30) << 0 << '\n';

      std::ofstream rgba_rfile_int;
      rgba_rfile_int.open ("Coupled Adaptive 0_001 INT.txt");
      a_shii.file_int = &rgba_rfile_int;

      rgba_rfile_int << std::setprecision (30) << 0 << '\t'
        << std::setprecision (30) << 0 << '\t'
        << std::setprecision (30) << 0 << '\t'
        << std::setprecision (30) << 0 << '\t'
        << std::setprecision (30) << 0 << '\t'
        << std::setprecision (30) << 0 << '\n';
#endif
      double s1 = lqc::Distance (minpos, maxpos);
      a_shii.CoupledIntegration (0, s1, 0.001, INTEGRATOR_INITIAL_STEP, m_min_h_step, m_max_h_step);
#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
      rgba_rfile_ext.close ();
      rgba_rfile_int.close ();
#endif
    }
    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
      {
        a_shii.Init (lqc::Vector3d (minpos.x, minpos.y, minpos.z), lqc::Vector3d (maxpos.x, maxpos.y, maxpos.z), m_volume, m_transfer_function);
#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
        std::ofstream rgba_rfile_ext;
        rgba_rfile_ext.open ("Decoupled Adaptive 0_01 EXT.txt");
        a_shii.file_ext = &rgba_rfile_ext;

        rgba_rfile_ext << std::setprecision (30) << 0 << '\t'
          << std::setprecision (30) << 0 << '\t'
          << std::setprecision (30) << 0 << '\t'
          << std::setprecision (30) << 0 << '\t'
          << std::setprecision (30) << 0 << '\t'
          << std::setprecision (30) << 0 << '\n';

        std::ofstream rgba_rfile_int;
        rgba_rfile_int.open ("Decoupled Adaptive 0_01 INT.txt");
        a_shii.file_int = &rgba_rfile_int;

        rgba_rfile_int << std::setprecision (30) << 0 << '\t'
          << std::setprecision (30) << 0 << '\t'
          << std::setprecision (30) << 0 << '\t'
          << std::setprecision (30) << 0 << '\t'
          << std::setprecision (30) << 0 << '\t'
          << std::setprecision (30) << 0 << '\n';
#endif
        double s1 = lqc::Distance (minpos, maxpos);
        a_shii.DecoupledIntegration (0, s1, 0.01, INTEGRATOR_INITIAL_STEP, m_min_h_step, m_max_h_step);
#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
        rgba_rfile_ext.close ();
        rgba_rfile_int.close ();
#endif
      }
      //////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////
    {
      a_shii.Init (lqc::Vector3d (minpos.x, minpos.y, minpos.z), lqc::Vector3d (maxpos.x, maxpos.y, maxpos.z), m_volume, m_transfer_function);
#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
      std::ofstream rgba_rfile_ext;
      rgba_rfile_ext.open ("Decoupled Adaptive 0_001 EXT.txt");
      a_shii.file_ext = &rgba_rfile_ext;

      rgba_rfile_ext << std::setprecision (30) << 0 << '\t'
        << std::setprecision (30) << 0 << '\t'
        << std::setprecision (30) << 0 << '\t'
        << std::setprecision (30) << 0 << '\t'
        << std::setprecision (30) << 0 << '\t'
        << std::setprecision (30) << 0 << '\n';

      std::ofstream rgba_rfile_int;
      rgba_rfile_int.open ("Decoupled Adaptive 0_001 INT.txt");
      a_shii.file_int = &rgba_rfile_int;

      rgba_rfile_int << std::setprecision (30) << 0 << '\t'
        << std::setprecision (30) << 0 << '\t'
        << std::setprecision (30) << 0 << '\t'
        << std::setprecision (30) << 0 << '\t'
        << std::setprecision (30) << 0 << '\t'
        << std::setprecision (30) << 0 << '\n';
#endif
      double s1 = lqc::Distance (minpos, maxpos);
      a_shii.DecoupledIntegration (0, s1, 0.001, INTEGRATOR_INITIAL_STEP, m_min_h_step, m_max_h_step);
#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
      rgba_rfile_ext.close ();
      rgba_rfile_int.close ();
#endif
    }
    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////


  return lqc::Vector4d (0);
}