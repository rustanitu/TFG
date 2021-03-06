#include "SimpsonErrorHalfIntegrator.h"

#include "VolumeEvaluator.h"

///Reutilizar n�o apenas amostras, mas tamb�m avalia��es de intervalos
int SimpsonErrorHalfIntegrator::s_IntegrativeExternalTimes  = 0;
int SimpsonErrorHalfIntegrator::s_IntegrativeInternalTimes = 0;

int SimpsonErrorHalfIntegrator::s_RecalculateIntegrativeExternalTimes = 0;
int SimpsonErrorHalfIntegrator::s_RecalculateIntegrativeInternalTimes = 0;

bool SimpsonErrorHalfIntegrator::ColorErrorEvalFunc (lqc::Vector4d a, lqc::Vector4d b, double tol)
{
  tol = 15.0 * tol;

  double error_r = fabs (a.x - b.x);
  double error_g = fabs (a.y - b.y);
  double error_b = fabs (a.z - b.z);
  double error_a = fabs (a.w - b.w);

#ifdef ANALYSIS__ERROR_ALONG_THE_RAY
  ext_aux_error = lqc::Vector4d (error_r, error_g, error_b, error_a);
#endif

  external_error_aux = lqc::Vector4d (error_r, error_g, error_b, error_a);

  return (tol >= error_r && tol >= error_g
    && tol >= error_b && tol >= error_a);
}

bool SimpsonErrorHalfIntegrator::AlphaErrorEvalFunc (double a, double b, double tol)
{
  tol = 15.0 * tol;
  double alphaerror = fabs (b - a);

#ifdef ANALYSIS__ERROR_ALONG_THE_RAY
  int_aux_error = alphaerror;
#endif

  internal_error_aux = alphaerror;
  return tol >= alphaerror;
}

SimpsonErrorHalfIntegrator::SimpsonErrorHalfIntegrator (VolumeEvaluator* veva)
{
  volume_evaluator = veva;
}

SimpsonErrorHalfIntegrator::~SimpsonErrorHalfIntegrator ()
{}

void SimpsonErrorHalfIntegrator::Init (lqc::Vector3d minp, lqc::Vector3d maxp, vr::Volume* vol, vr::TransferFunction* tf)
{
  volume = vol;
  transfer_function = tf;

  minpos = minp;
  maxpos = maxp;
  normalized_step = lqc::Vector3d::Normalize (maxpos - minpos);

  pre_integrated = 0.0;
  color = lqc::Vector4d (0);
}

void SimpsonErrorHalfIntegrator::Integrate (double s0, double s1, double tol, double h0)
{
  double v[3] = { -1, -1, -1 };
  lqc::Vector4d clr[5];
  double s = s0;

#ifdef ANALYSIS__ERROR_ALONG_THE_RAY
  error_along_the_ray.m_int_errorintervals.clear ();
  error_along_the_ray.m_ext_errorintervals.clear ();
#endif

  clr[0] = GetFromTransferFunction (s);
  while (s < s1)
  {
    internal_sum_error = 0.0;
    external_sum_error = lqc::Vector4d (0.0);

    double h = AdaptiveInternalIntegration (s, h0, (tol * h0) / (s1 - s0), v, clr);
    double max_h_tol = (tol * h) / (s1 - s0);
    if (!TryExternaIntegration (s, h, max_h_tol, v, clr))
      AdaptiveExternalIntegration (s, h, max_h_tol, &pre_integrated);

    s = s + h;
    
    double extsumerror = MaxExternalError (external_sum_error);
    if (extsumerror > internal_sum_error)
      if (extsumerror == 0)  h = h == h0 ? 2.0 * h : h;
      else h = sqrt (sqrt (max_h_tol*MAX_ERROR_FACTOR / extsumerror)) * h;
    else
      if (internal_sum_error == 0)  h = h == h0 ? 2.0 * h : h;
      else h = sqrt (sqrt (max_h_tol*MAX_ERROR_FACTOR / internal_sum_error)) * h;
    
    h0 = MIN (h, s1 - s);
#ifdef INTEGRATOR__STEP_CONTROLLED
    h0 = MIN (h0, INTEGRATOR__MAX_STEP_BOUNDARY);
#endif

    pre_integrated += (v[1] + v[2] + (v[1] + v[2] - v[0]) / 15.0);
    clr[0] = clr[4];
  }
#ifdef ANALYSIS__ERROR_ALONG_THE_RAY
  error_along_the_ray.PrintResults ("AdaptiveSimpsonErrorIntegrator_InternalSimpson.txt",
    "AdaptiveSimpsonErrorIntegrator_ExternalSimpson.txt");
#endif
}

bool SimpsonErrorHalfIntegrator::IntegrateInternalInterval (double a, double b, double tol, double* pS, double* pSleft, double* pSright, lqc::Vector4d clr[], bool force)
{
#ifdef ANALYSIS__STORE_INTEGRATE_INTERVAL_TIMES
  SimpsonErrorHalfIntegrator::s_IntegrativeInternalTimes++;
#endif

  double h = (b - a);

  double fa = clr[0].w;
  double fd = clr[1].w;
  double fc = clr[2].w;
  double fe = clr[3].w;
  double fb = clr[4].w;

  double S = SimpsonRule<double> (fa, fc, fb, h);
  double Sleft = SimpsonRule<double> (fa, fd, fc, h / 2.0); 
  double Sright = SimpsonRule<double> (fc, fe, fb, h / 2.0);
  double S2 = Sleft + Sright;

  if (AlphaErrorEvalFunc (S2, S, tol) || force)
  {
    (*pS) = S;
    (*pSright) = Sright;
    (*pSleft) = Sleft;

    return true;
  }
  return false;
}

double SimpsonErrorHalfIntegrator::AdaptiveInternalIntegration (double s, double h0, double tol, double values[], lqc::Vector4d clr[])
{
  double h = h0;
  double scl_error;
  lqc::Vector4d faux[5];

  double a, b, c, d, e;
  a = s; b = s + h; c = (a + b) / 2.0;
  faux[0] = clr[0];
  
  scl_error = (tol * h) / h0;

  a = s; b = s + h; c = (a + b) / 2.0;
  d = (a + c) / 2.0; e = (c + b) / 2.0;
  faux[1] = GetFromTransferFunction (d);
  faux[2] = GetFromTransferFunction (c);
  faux[3] = GetFromTransferFunction (e);
  faux[4] = GetFromTransferFunction (b);

  if (!IntegrateInternalInterval (s, s + h, scl_error, &values[0], &values[1], &values[2], faux, false))
  {

#ifdef ANALYSIS__STORE_INTEGRATE_INTERVAL_TIMES
    SimpsonErrorHalfIntegrator::s_RecalculateIntegrativeInternalTimes++;
#endif

    h = sqrt (sqrt (scl_error*MAX_ERROR_FACTOR / internal_error_aux)) * h;

    scl_error = (tol * h) / h0;

    a = s; b = s + h; c = (a + b) / 2.0;
    d = (a + c) / 2.0; e = (c + b) / 2.0;
    faux[1] = GetFromTransferFunction (d);
    faux[2] = GetFromTransferFunction (c);
    faux[3] = GetFromTransferFunction (e);
    faux[4] = GetFromTransferFunction (b);

    IntegrateInternalInterval (s, s + h, scl_error, &values[0], &values[1], &values[2], faux, true);
  }
#ifdef ANALYSIS__ERROR_ALONG_THE_RAY
  int_left_error = int_aux_error;
#endif

  clr[0] = faux[0]; clr[1] = faux[1]; clr[2] = faux[2]; clr[3] = faux[3]; clr[4] = faux[4];
  internal_sum_error = internal_error_aux;

#ifdef ANALYSIS__ERROR_ALONG_THE_RAY
  error_along_the_ray.m_int_errorintervals.push_back (Analysis_ErrorAlongTheRay::Int_IntervalError (s, h, int_left_error));
#endif

  return h;
}

lqc::Vector4d SimpsonErrorHalfIntegrator::ExternalIntegration (lqc::Vector4d C, double p_d, double inner)
{
  double innerint = exp (-(pre_integrated + inner));

  double alphachannel = C.w*innerint;
  return lqc::Vector4d (
    alphachannel * C.x,
    alphachannel * C.y,
    alphachannel * C.z,
    alphachannel
    );
}

bool SimpsonErrorHalfIntegrator::TryExternaIntegration (double s, double h, double tol, double v[], lqc::Vector4d clr[])
{
#ifdef ANALYSIS__STORE_INTEGRATE_INTERVAL_TIMES
  SimpsonErrorHalfIntegrator::s_IntegrativeExternalTimes++;
#endif

  double a = s, c = (s + h) / 2.0, b = s + h;
  double d = (a + c) / 2.0, e = (c + b) / 2.0;

  lqc::Vector4d fa = ExternalIntegration (clr[0], s, 0);
  lqc::Vector4d fd = ExternalIntegration (clr[1], s + (h / 4.0), SimpsonRule<double> (clr[0].w, GetFromTransferFunction (s + (h / 8.0)).w, clr[1].w, (h / 4.0)));
  lqc::Vector4d fc = ExternalIntegration (clr[2], s + (h / 2.0), v[1]);
  lqc::Vector4d fe = ExternalIntegration (clr[3], s + (3.0 * h / 4.0), v[1] + SimpsonRule<double> (clr[2].w, GetFromTransferFunction (s + (5.0 * h / 8.0)).w, clr[3].w, (h / 4.0)));
  lqc::Vector4d fb = ExternalIntegration (clr[4], s + h, (v[1] + v[2] + (v[1] + v[2] - v[0]) / 15.0));

  lqc::Vector4d S = (h / 6.0) * (fa + (4.0 * fc) + fb);
  lqc::Vector4d Sleft = (h / 12.0) * (fa + (4.0 * fd) + fc);
  lqc::Vector4d Sright = (h / 12.0) * (fc + (4.0 * fe) + fb);
  lqc::Vector4d S2 = Sleft + Sright;

  if (ColorErrorEvalFunc (S, S2, tol))
  {
    lqc::Vector4d Sret = S2 + (S2 - S) / 15.0;
    color += Sret;

    external_sum_error += external_error_aux;

#ifdef ANALYSIS__ERROR_ALONG_THE_RAY
    error_along_the_ray.m_ext_errorintervals.push_back (Analysis_ErrorAlongTheRay::Ext_IntervalError (s, h, ext_aux_error));
#endif

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
    ExternalCumulativeError exce;
    exce.s = s + h;
    exce.h = h;
    exce.err = color;

    m_excumulative_error.push_back (exce);
#endif

    return true;
  }
  return false;
}

void SimpsonErrorHalfIntegrator::AdaptiveExternalIntegration (double s, double h, double tol, double* pre_integrated)
{
  minpost = s;
  Cminpost = GetFromTransferFunction (s);
  
  double d = h;
  double s1 = s + h;
  
  last_color = Cminpost;
  
  while (s < s1)
    s += Aux_AdaptiveExternalIntegration (s, s1 - s, (tol * fabs (s1 - s)) / d, pre_integrated);
}

lqc::Vector4d SimpsonErrorHalfIntegrator::ExtenalEvaluation (double p_d, lqc::Vector4d C)
{
  double alpha = ((p_d - minpost) / 6.0) * (Cminpost.w +
    4.0 * GetFromTransferFunction ((minpost + p_d) / 2.0).w + C.w);
  return ExternalIntegration (C, p_d, alpha);
}

bool SimpsonErrorHalfIntegrator::IntegrateExternalInterval (double a, double b, double tol, lqc::Vector4d* pS, lqc::Vector4d clr[], bool force)
{
#ifdef ANALYSIS__STORE_INTEGRATE_INTERVAL_TIMES
  SimpsonErrorHalfIntegrator::s_IntegrativeExternalTimes++;
#endif
 
  double h = (b - a), c = (a + b) / 2.0,
    d = (a + c) / 2.0, e = (c + b) / 2.0;
  lqc::Vector4d fa = ExtenalEvaluation (a, clr[0]);
  lqc::Vector4d fd = ExtenalEvaluation (d, clr[1]);
  lqc::Vector4d fc = ExtenalEvaluation (c, clr[2]);
  lqc::Vector4d fe = ExtenalEvaluation (e, clr[3]);
  lqc::Vector4d fb = ExtenalEvaluation (b, clr[4]);

  lqc::Vector4d S = SimpsonRule<lqc::Vector4d> (fa, fc, fb, h);
  lqc::Vector4d Sleft = SimpsonRule<lqc::Vector4d> (fa, fd, fc, h / 2.0);
  lqc::Vector4d Sright = SimpsonRule<lqc::Vector4d> (fc, fe, fb, h / 2.0);
  lqc::Vector4d S2 = Sleft + Sright;

  if (ColorErrorEvalFunc (S, S2, tol) || force)
  {
    (*pS) = S2 + (S2 - S) / 15.0;
    return true;
  }
  return false;
}

double SimpsonErrorHalfIntegrator::Aux_AdaptiveExternalIntegration (double s, double h0, double tol, double* pre_integrated)
{
  double h = h0;
  double scl_error;

  lqc::Vector4d fleft[5];
  lqc::Vector4d ret_color;

  double a, b, c, d, e;
  a = s; b = s + h; c = (a + b) / 2.0;
  fleft[0] = last_color;

  scl_error = (tol * h) / h0;

  a = s; b = s + h; c = (a + b) / 2.0;
  d = (a + c) / 2.0; e = (c + b) / 2.0;
  
  fleft[1] = GetFromTransferFunction (d);
  fleft[2] = GetFromTransferFunction (c);
  fleft[3] = GetFromTransferFunction (e);
  fleft[4] = GetFromTransferFunction (b);

  if (!IntegrateExternalInterval (s, s + h, scl_error, &ret_color, fleft, false))
  {
#ifdef ANALYSIS__STORE_INTEGRATE_INTERVAL_TIMES
    SimpsonErrorHalfIntegrator::s_RecalculateIntegrativeExternalTimes++;
#endif

    h = sqrt (sqrt (scl_error*MAX_ERROR_FACTOR / MaxExternalError (external_error_aux))) * h;
    scl_error = (tol * h) / h0;

    a = s; b = s + h; c = (a + b) / 2.0;
    d = (a + c) / 2.0; e = (c + b) / 2.0;
    fleft[1] = GetFromTransferFunction (d);
    fleft[2] = GetFromTransferFunction (c);
    fleft[3] = GetFromTransferFunction (e);
    fleft[4] = GetFromTransferFunction (b);

    IntegrateExternalInterval (s, s + h, scl_error, &ret_color, fleft, true);
  }
#ifdef ANALYSIS__ERROR_ALONG_THE_RAY
  ext_left_error = ext_aux_error;
#endif

  color += ret_color;
  external_sum_error += external_error_aux;

#ifdef ANALYSIS__ERROR_ALONG_THE_RAY
  error_along_the_ray.m_ext_errorintervals.push_back (Analysis_ErrorAlongTheRay::Ext_IntervalError (s, h, ext_left_error));
#endif

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
  ExternalCumulativeError exce;
  exce.s = s + h;
  exce.h = h;
  exce.err = color;
  m_excumulative_error.push_back (exce);
#endif

  last_color = fleft[4];
  
  return h;
}

lqc::Vector4d SimpsonErrorHalfIntegrator::GetFromTransferFunction (double p_d)
{
  lqc::Vector4d ret;
  lqc::Vector3d p = minpos + p_d * normalized_step;
  if (!transfer_function || !volume) ret = lqc::Vector4d (0.0);
  else ret = transfer_function->Get (volume_evaluator->GetValueFromVolume (volume, lqc::Vector3f (p.x, p.y, p.z)));
  return ret;
}