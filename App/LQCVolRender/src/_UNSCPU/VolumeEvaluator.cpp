#include "VolumeEvaluator.h"

#include <lqc/Math/MGeometry.h>
#include <lqc/Math/Vector3.h>
#include <lqc/Math/Vector4.h>

#include <lqc/lqcdefines.h>
#include <lqc/Utils/Utils.h>

#define STATISTICS

// VolumeEvaluator

VolumeEvaluator::VolumeEvaluator ()
: m_evaluate_function_data (NULL),
m_use_evaluate_function (false),
m_volume (NULL),
m_alpha_truncation (false)
{
  m_current_method = C_RIEMANN_RGBA;
  m_transfer_function = NULL;
  m_adaptive_simpson_max_depth = 100;
  m_alpha_max_breaker = 1.0f;
  m_integral_error = 1.0;
  m_error_inner_integral = 1.0;
  m_adaptive_distance_intervals = 1.0;
  m_opacity_correction = 1.0f;
  m_distance_intervals = 1.0;
}

VolumeEvaluator::~VolumeEvaluator ()
{}

void VolumeEvaluator::SetFunctionEvaluationIndex (int index)
{
  m_voleval_functions.SetIndexFunc (index);
}

void VolumeEvaluator::SetFunctionEvaluationData (void *data)
{
  m_evaluate_function_data = data;
}

void VolumeEvaluator::CleanFunctionEvaluationData ()
{
  m_evaluate_function_data = NULL;
}

void VolumeEvaluator::SetUseEvaluationFunction (bool b)
{
  m_use_evaluate_function = b;
}

void VolumeEvaluator::SetModelAndTransferFunction (lqc::Volume* ivolume, lqc::TransferFunction* itransfer_function)
{
  m_volume = ivolume;
  m_transfer_function = itransfer_function;
}

bool VolumeEvaluator::EvaluateIntegral (lqc::Vector4d *returncolor, lqc::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos)
{
  *returncolor = lqc::Vector4d (-1.f);
  if (!volume)
    return false;

  switch (m_current_method)
  {
    case C_RIEMANN_R:
      m_composition_method = COMPOSITION_METHODS::C_R;
      *returncolor = Riemann (volume, minpos, maxpos);
      break;
    case C_RIEMANN_RA:
      if (!m_transfer_function) return false;
      m_composition_method = COMPOSITION_METHODS::C_RA;
      *returncolor = Riemann (volume, minpos, maxpos);
      break;
    case C_RIEMANN_RGBA:
      if (!m_transfer_function) return false;
      m_composition_method = COMPOSITION_METHODS::C_RGBA;
      *returncolor = Riemann (volume, minpos, maxpos);
      break;
    case I_RIEMANN:
      if (!m_transfer_function) return false;
      *returncolor = Riemann_I (volume, minpos, maxpos);
      break;
    case I_ADAPTIVESIMPSONPARTICIONED:
      if (!m_transfer_function) return false;
      *returncolor = AdaptiveSimpson_I (volume, minpos, maxpos);
      break;
  }
  return true;
}

lqc::Vector4d VolumeEvaluator::TransferFunction (double value)
{
  if (!m_transfer_function) return lqc::Vector4d (-1.f);
#ifdef STATISTICS
  m_statistics.n_gets_tf_values++;
#endif
  return m_transfer_function->Get (value);
}

float VolumeEvaluator::GetValueFromVolume (lqc::Volume* volume, lqc::Vector3f pos)
{
#ifdef STATISTICS
  m_statistics.n_gets_volume_values++;
#endif
  if (!m_use_evaluate_function)
    return volume->InterpolatedValue (pos);
  else
    return m_voleval_functions.F (lqc::Vector3d (pos.x, pos.y, pos.z), m_evaluate_function_data);
}

void VolumeEvaluator::Composite (lqc::Vector4d* color, float value, float stepdistance, lqc::Vector3f minpos, lqc::Vector3f evaluatedpos, bool first)
{
#ifdef STATISTICS
  m_statistics.n_compositions++;
#endif
  lqc::Vector4d src;
  if (m_composition_method == COMPOSITION_METHODS::C_R)
  {
    src = lqc::Vector4d (value / 255.f);
    src.w *= 0.5f;
  }
  else if (m_composition_method == COMPOSITION_METHODS::C_RA)
  {
    src = lqc::Vector4d (value / 255.f);
    src.w = TransferFunction (value).w;
  }
  else if (m_composition_method == COMPOSITION_METHODS::C_RGBA)
  {
    lqc::Vector4d tfsrc = TransferFunction (value);
    src = lqc::Vector4d (tfsrc.x, tfsrc.y, tfsrc.z, tfsrc.w);
  }
  src.w = 1 - pow (1 - src.w, stepdistance / m_opacity_correction);
  if (first)
    (*color) = src;
  else
  {
    //Front to back blending - Optical Models for Volume Rendering
    // dst.rgb = dst.rgb + (1 - dst.a) * src.a * src.rgb
    // dst.a   = dst.a   + (1 - dst.a) * src.a     
    src.x *= src.w; src.y *= src.w; src.z *= src.w;
    (*color) = (1.0f - (*color).w) * src + (*color);
    //NVIDIA http://http.developer.nvidia.com/GPUGems/gpugems_ch39.html
    //(*color) = (1.0f - (*color).w) * src + (*color);
  }
}

lqc::Vector4d VolumeEvaluator::Riemann (lqc::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos)
{
  lqc::Vector4d color (0);

  //Composite (&color, GetValueFromVolume (volume, minpos), stepdistance, true);

  double distminmax = lqc::Distance (minpos, maxpos);
  lqc::Vector3f Step = lqc::Vector3f::Normalize (maxpos - minpos)*m_distance_intervals;
  lqc::Vector3f pos = minpos + Step;
  while (lqc::Distance (minpos, pos) <= distminmax)
  {
    Composite (&color, GetValueFromVolume (volume, pos), m_distance_intervals, minpos, pos);
    if (color.w >= m_alpha_max_breaker) break;
    pos += Step;
  }
  pos -= Step;
  if (lqc::Distance (pos, maxpos) > KEPSILON && color.w < m_alpha_max_breaker)
    Composite (&color, GetValueFromVolume (volume, maxpos), lqc::Distance (pos, maxpos), minpos, pos);

  return color;
}

lqc::Vector4d VolumeEvaluator::Riemann_I (lqc::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos)
{
  lqc::Vector4d color (0);

  double distminmax = lqc::Distance (minpos, maxpos);
  lqc::Vector3f Step = lqc::Vector3f::Normalize (maxpos - minpos) * m_distance_intervals;

  lqc::Vector3f p1 = minpos;
  lqc::Vector3f p2 = p1 + Step;
  
  lqc::Vector4d fsrc;
  lqc::Vector4d src;
  double innerintegral = 0;
  while (lqc::Distance (minpos, p2) <= distminmax)
  {
    fsrc = TransferFunction (GetValueFromVolume (volume, p2));
    src = lqc::Vector4d (fsrc.x, fsrc.y, fsrc.z, fsrc.w);

    innerintegral += src.w * m_distance_intervals;

    double inn = exp (-innerintegral);

    color.x += src.x * src.w * m_distance_intervals * inn;
    color.y += src.y * src.w * m_distance_intervals * inn;
    color.z += src.z * src.w * m_distance_intervals * inn;
    color.w += src.w * m_distance_intervals * inn;

    p1 = p2;
    p2 = p1 + Step;
  }
  p2 = maxpos;
  if (lqc::Distance (p1, p2) > KEPSILON)
  {
    float d = lqc::Distance (p1, p2);
    fsrc = TransferFunction (GetValueFromVolume (volume, p2));
    src = lqc::Vector4d (fsrc.x, fsrc.y, fsrc.z, fsrc.w);

    innerintegral += src.w * d;

    double inn = exp (-innerintegral);

    color.x += src.x * src.w * d * inn;
    color.y += src.y * src.w * d * inn;
    color.z += src.z * src.w * d * inn;
    color.w += src.w * d * inn;
  }

  if (m_alpha_truncation)
    color.w = m_alpha_max_breaker;

  return color;
}

lqc::Vector4d VolumeEvaluator::AdaptiveSimpson_I (lqc::Volume *volume, lqc::Vector3f minpos, lqc::Vector3f maxpos)
{
  SimpsonEvaluator<lqc::Vector4d, lqc::Vector3d> se;
  se.max_interval = m_distance_intervals;

  lqc::Vector4d color (0);

  //Create a EvaluationIntegral
  EvaluationIntegral data(this);
  data.volume = volume;
  data.transferFunction = m_transfer_function;
  data.maxdepth = m_adaptive_simpson_max_depth;
  data.minpos = lqc::Vector3d (minpos.x, minpos.y, minpos.z);
  data.rec_minpos = data.minpos;
  data.error = m_integral_error;
  data.ierror = m_error_inner_integral;
  data.m_minpos = lqc::Vector3d (minpos.x, minpos.y, minpos.z);
  data.m_maxpos = lqc::Vector3d (maxpos.x, maxpos.y, maxpos.z);
  data.m_normalized_step = lqc::Vector3d::Normalize (data.m_maxpos - data.m_minpos);
  data.optimization = true;

  se.SetReturnFunction (EvaluationIntegral::whenreturn);
  se.SetBeforeRecursionFunction (EvaluationIntegral::beforerecursion);

  //calcula a distância total da avaliação
  double distminmax = lqc::Distance (minpos, maxpos);
  //Determina o tamanho de cada intervalo baseado no número de intervalos que serão avaliados
  lqc::Vector3f Step = lqc::Vector3f::Normalize (maxpos - minpos)*m_adaptive_distance_intervals;

  //primeiros pontos inicial e final de um intervalo
  lqc::Vector3f p1 = minpos;
  lqc::Vector3f p2 = p1 + Step;

  //Cálcula o valor proporcional do erro (sendo o erro como erro/1 de distância)
  double errorbound = m_integral_error;

  lqc::Vector4d tb = TransferFunction (GetValueFromVolume (volume, p1));
  lqc::Vector4d tinit = tb;
  while (lqc::Distance (minpos, p2) <= distminmax)
  {
    data.inner_counter = 0;
    data.stay_1_inner_counter = true;
    color += se.AdaptiveSimpsons (EvaluationIntegral::EvaluateVolumeRenderingIntergral,
                                  lqc::Vector3d (p1.x, p1.y, p1.z), lqc::Vector3d (p2.x, p2.y, p2.z), errorbound,
                                  m_adaptive_simpson_max_depth, EvaluationIntegral::colorErrorEvaluationFunc,
                                  EvaluationIntegral::distanceVector3dfunc, &data);

    p1 = p2;
    p2 = p1 + Step;
  }
  p2 = maxpos;
  if (lqc::Distance (p1, p2) > KEPSILON)
  {
    data.inner_counter = 0;
    data.stay_1_inner_counter = true;
    color += se.AdaptiveSimpsons (EvaluationIntegral::EvaluateVolumeRenderingIntergral,
                                  lqc::Vector3d (p1.x, p1.y, p1.z), lqc::Vector3d (p2.x, p2.y, p2.z), errorbound,
                                  m_adaptive_simpson_max_depth, EvaluationIntegral::colorErrorEvaluationFunc,
                                  EvaluationIntegral::distanceVector3dfunc, &data);
  }

  if (color.w > m_alpha_max_breaker)
    color.w = m_alpha_max_breaker;

  if (m_alpha_truncation)
    color.w = m_alpha_max_breaker;
  return color;
}

// VolumeEvaluator*

// EvaluationIntegral

/////////////////////
// public methods  //
/////////////////////

EvaluationIntegral::EvaluationIntegral (VolumeEvaluator* veva)
: m_volumeevaluator (veva),
rec_current_opacity_eval (0.0),
pre_computed_opacity (0.0),
optimization (true),
inner_counter (0),
stay_1_inner_counter (true)
{
}

EvaluationIntegral::~EvaluationIntegral ()
{
}

double EvaluationIntegral::EvaluateVolumeRenderingInnerIntergral (lqc::Vector3d p, double h, void* data)
{
  EvaluationIntegral* d = (EvaluationIntegral*)data;
  return d->GetOpacity (d->GetFromTransferFunction (p));
}

lqc::Vector4d EvaluationIntegral::EvaluateVolumeRenderingIntergral (lqc::Vector3d p, double h, void* data)
{
  EvaluationIntegral* d = (EvaluationIntegral*)data;
  return d->Evaluation (p, h, data);
}

double EvaluationIntegral::distanceVector3dfunc (lqc::Vector3d a, lqc::Vector3d b)
{
  if (a == b)
    return 0.0;

  return sqrt(((b.x - a.x)*(b.x - a.x)) +
              ((b.y - a.y)*(b.y - a.y)) +
              ((b.z - a.z)*(b.z - a.z)));

  /*return (b.x - a.x) + (b.y - a.y) + (b.z - a.z);*/
}

double EvaluationIntegral::distanceDoublefunc (double a, double b)
{
  return lqc::Distance (a, b);
}

double EvaluationIntegral::colorErrorEvaluationFunc (lqc::Vector4d a, lqc::Vector4d b)
{
  return lqc::Distance (lqc::Vector3d (a.x, a.y, a.z),
    lqc::Vector3d (b.x, b.y, b.z));
}

lqc::Vector4d EvaluationIntegral::Evaluation (lqc::Vector3d p, double h, void* data)
{
  SimpsonEvaluator<double, lqc::Vector3d> se;

  lqc::Vector4d C = GetFromTransferFunction (p);
  double tau = GetOpacity (C);

  double ois = 0.0;
  double err = ierror;
  double innerint = 0.0;
  // OPTIMIZATION
  if (optimization)
  {
    if (err > 0.0)
    {
      lqc::Vector3d pi = rec_minpos;
      lqc::Vector3d step = m_normalized_step * h;
      lqc::Vector3d pf = pi + step;

      for (int i = 0; i < inner_counter; i++)
        //while (lqc::Distance (rec_minpos, pf) <= lqc::Distance (rec_minpos, p))
      {
        ois += se.AdaptiveSimpsons (EvaluationIntegral::EvaluateVolumeRenderingInnerIntergral,
          pi, pf, err, maxdepth, EvaluationIntegral::distanceDoublefunc,
          EvaluationIntegral::distanceVector3dfunc, this);
        pi = pf;
        pf = pi + step;
      }
      rec_current_opacity_eval += ois;
    }

    if (stay_1_inner_counter)
      inner_counter = 1;
    else
      inner_counter++;

    rec_minpos = p;

    innerint = exp (-(rec_current_opacity_eval + pre_computed_opacity));
  }
  // OPTIMIZATION
  // OLD
  else
  {
    if (err > 0.0)
    {
      lqc::Vector3d pi = m_minpos;
      lqc::Vector3d step = m_normalized_step * h;
      lqc::Vector3d pf = pi + step;
      while (lqc::Distance (m_minpos, pf) <= lqc::Distance (m_minpos, p))
      {
        ois += se.AdaptiveSimpsons (EvaluationIntegral::EvaluateVolumeRenderingInnerIntergral,
          pi, pf, err, maxdepth, EvaluationIntegral::distanceDoublefunc,
          EvaluationIntegral::distanceVector3dfunc, this);
        pi = pf;
        pf = pi + step;
      }
    }
    innerint = exp (-ois);
  }
  // OLD

  return lqc::Vector4d (C.x*tau*innerint, C.y*tau*innerint, C.z*tau*innerint, tau*innerint);
}

void EvaluationIntegral::whenreturn (lqc::Vector4d clr, lqc::Vector3d a, lqc::Vector3d b, void* data)
{
  EvaluationIntegral* d = (EvaluationIntegral*)data;
  SimpsonEvaluator<double, lqc::Vector3d> se;
  se.max_interval = 0.0;

  // OPTIMIZATION
  if (d->optimization)
  {
    if (d->ierror > 0.0)
    {
      double ois = se.AdaptiveSimpsons (EvaluationIntegral::EvaluateVolumeRenderingInnerIntergral,
        d->rec_minpos, b, d->ierror, d->maxdepth, EvaluationIntegral::distanceDoublefunc,
        EvaluationIntegral::distanceVector3dfunc, data);
      d->rec_minpos = b;
      d->rec_current_opacity_eval += ois;
    }
    d->rec_minpos = b;
    d->pre_computed_opacity += d->rec_current_opacity_eval;
    d->minpos = b;
  }
  // OPTIMIZATION
}

void EvaluationIntegral::beforerecursion (void* data, double error)
{
  EvaluationIntegral* d = (EvaluationIntegral*)data;
  // OPTIMIZATION
  if (d->optimization)
  {
    d->rec_current_opacity_eval = 0.0;
    d->rec_minpos = d->minpos;
    d->inner_counter = 1;
    d->stay_1_inner_counter = false;
  }
  // OPTIMIZATION
}

/////////////////////
//protected methods//
/////////////////////

/////////////////////
// private methods //
/////////////////////

double EvaluationIntegral::GetOpacity (lqc::Vector4d color)
{
  return color.w;
}

lqc::Vector4d EvaluationIntegral::GetFromTransferFunction (lqc::Vector3d p)
{
  if (!transferFunction || !volume) return lqc::Vector4d (0.0f);
  return transferFunction->Get (m_volumeevaluator->GetValueFromVolume (volume, lqc::Vector3f (p.x, p.y, p.z)));
}

// EvaluationIntegral*