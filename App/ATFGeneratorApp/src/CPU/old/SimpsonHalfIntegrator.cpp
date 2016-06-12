#include "SimpsonHalfIntegrator.h"

#include "VolumeEvaluator.h"

///Reutilizar não apenas amostras, mas também avaliações de intervalos
double AdaptiveSimpsonHalfIntegrator::s_acumulated_internal_error = 0.0;
double AdaptiveSimpsonHalfIntegrator::s_acumulated_external_error = 0.0;
int AdaptiveSimpsonHalfIntegrator::s_IntegrativeExternalTimes  = 0;
int AdaptiveSimpsonHalfIntegrator::s_IntegrativeInternalTimes  = 0;

int AdaptiveSimpsonHalfIntegrator::s_RecalculateIntegrativeExternalTimes = 0;
int AdaptiveSimpsonHalfIntegrator::s_RecalculateIntegrativeInternalTimes = 0;

AdaptiveSimpsonHalfIntegrator::AdaptiveSimpsonHalfIntegrator (lqc::Vector3d minp, 
                           lqc::Vector3d maxp, VolumeEvaluator* veva,
                           vr::Volume* vol, vr::TransferFunction* tf)
{
  volume_evaluator = veva;
  volume = vol;
  transfer_function = tf;

  minpos = minp;
  maxpos = maxp;
  normalized_step = lqc::Vector3d::Normalize (maxpos - minpos);

  pre_integrated = 0.0;
  color = lqc::Vector4d (0);
}

AdaptiveSimpsonHalfIntegrator::~AdaptiveSimpsonHalfIntegrator ()
{}

void AdaptiveSimpsonHalfIntegrator::Integrate (double s0, double s1, double tol, double h0)
{
  double v[6] = { -1, -1, -1, -1, -1, -1 };
  lqc::Vector4d clr[5];
  double s = s0;

  Integrate_InitAnalysisAuxVariables ();

  clr[0] = GetFromTransferFunction (s);
  while (s < s1)
  {
    Integrate_ResetAnalysisAuxVariables ();

    double h = AdaptiveInternalIntegration (s, h0, (tol * h0) / (s1 - s0), v, clr);
    double max_h_tol = (tol * h) / (s1 - s0);
    if (!TryExternaIntegration (s, h, max_h_tol, v, clr))
      AdaptiveExternalIntegration (s, h, max_h_tol, &pre_integrated);

    s = s + h;
    h0 = Integrate_CalculateNextInitialInterval (s, h, max_h_tol, h0, s1);

    pre_integrated += (v[2] + v[3] + (v[2] + v[3] - v[0]) / 15.0) +
                      (v[4] + v[5] + (v[4] + v[5] - v[1]) / 15.0);
    clr[0] = clr[4];
  }
#ifdef ANALYSIS__ERROR_ALONG_THE_RAY
  error_along_the_ray.PrintResults ("InternalSimpson.txt", "ExternalSimpson.txt");
#endif
}

float aint_left_S, aint_right_S;
bool AdaptiveSimpsonHalfIntegrator::IntegrateInternalInterval (double a, double b, double tol, double* pS, double* pSleft, double* pSright, lqc::Vector4d clr[], bool force, double Sprecalculated)
{
#ifdef ANALYSIS__STORE_INTEGRATE_INTERVAL_TIMES
  AdaptiveSimpsonHalfIntegrator::s_IntegrativeInternalTimes++;
#endif

  double h = (b - a);

  double fa = clr[0].w;
  double fd = clr[1].w;
  double fc = clr[2].w;
  double fe = clr[3].w;
  double fb = clr[4].w;

  double S = Sprecalculated >= 0 ? Sprecalculated : SimpsonRule<double> (fa, fc, fb, h);
  double Sleft = SimpsonRule<double> (fa, fd, fc, h / 2.0); 
  double Sright = SimpsonRule<double> (fc, fe, fb, h / 2.0);
  double S2 = Sleft + Sright;

  if (AlphaErrorEvalFunc (S2, S, tol) || force)
  {
    (*pSright) = Sright;
    (*pSleft) = Sleft;
    (*pS) = S;
    return true;
  }
  aint_left_S = Sleft;
  aint_right_S = Sright;
  return false;
}

double AdaptiveSimpsonHalfIntegrator::AdaptiveInternalIntegration (double s, double h0, double tol, double values[], lqc::Vector4d clr[])
{
#ifdef ANALYSIS__CALCULATE_ERROR_BEYOND_LIMIT
  InternalErrorBeyondLimit ebl_left;
  InternalErrorBeyondLimit ebl_right;
#endif

  double h = h0;
  double h_2 = h / 2.0;
  double scl_error;
  lqc::Vector4d fleft[5];
  lqc::Vector4d fright[5];

  aint_left_S = aint_right_S = -1;

  double a, b, c, d, e;
  
  a = s; b = s + h_2; c = (a + b) / 2.0;
  fleft[0] = clr[0];
  fleft[1] = GetFromTransferFunction (c);
  fleft[2] = GetFromTransferFunction (b);

  while (true)
  {
    scl_error = (tol * h_2) / h0;

    a = s; b = s + h_2; c = (a + b) / 2.0;
    d = (a + c) / 2.0; e = (c + b) / 2.0;
    fleft[4] = fleft[2];
    fleft[2] = fleft[1];
    fleft[1] = GetFromTransferFunction (d);
    fleft[3] = GetFromTransferFunction (e);

    if (IntegrateInternalInterval (s, s + h_2, scl_error, &values[0], &values[2], &values[3], fleft, false, aint_left_S))
    {
      InternalIntegration_SetLeftAuxVariables ();

#ifdef ANALYSIS__CALCULATE_ERROR_BEYOND_LIMIT
      (*ebl_left).error = internal_left_error_evaluated - scl_error > 0 ? internal_left_error_evaluated - scl_error : 0;
      (*ebl_left).s = s;
      (*ebl_left).h = h_2;
#endif
      break;
    }


#ifdef ANALYSIS__STORE_INTEGRATE_INTERVAL_TIMES
    AdaptiveSimpsonHalfIntegrator::s_RecalculateIntegrativeInternalTimes++;
#endif

    h /= 2.0;
    h_2 = h / 2.0;
  }

  a = s + h_2; b = s + h; c = (a + b) / 2.0;
  d = (a + c) / 2.0; e = (c + b) / 2.0;
  fright[0] = fleft[4];
  fright[1] = GetFromTransferFunction (d);
  fright[2] = GetFromTransferFunction (c);
  fright[3] = GetFromTransferFunction (e);
  fright[4] = GetFromTransferFunction (b);

  if (!IntegrateInternalInterval (s + h_2, s + h, scl_error, &values[1], &values[4], &values[5], fright, false, aint_right_S))
  {
    aint_left_S = values[2];
    aint_right_S = values[3];

    h /= 2.0;
    h_2 = h / 2.0;

    scl_error = (tol * h_2) / h0;

    //arrumar valores da segunda parte
    a = s + h_2; b = s + h; c = (a + b) / 2.0;
    fright[0] = fleft[2];
    fright[1] = GetFromTransferFunction ((a + c) / 2.0);
    fright[2] = fleft[3];
    fright[3] = GetFromTransferFunction ((c + b) / 2.0);
    fright[4] = fleft[4];
    //arrumar valores da primeira parte
    a = s; b = s + h_2; c = (a + b) / 2.0;
    fleft[4] = fleft[2];
    fleft[2] = fleft[1];
    fleft[1] = GetFromTransferFunction ((a + c) / 2.0);
    fleft[3] = GetFromTransferFunction ((c + b) / 2.0);
    
    IntegrateInternalInterval (s, s + h_2, scl_error, &values[0], &values[2], &values[3], fleft, true, aint_left_S);
    InternalIntegration_SetLeftAuxVariables ();

#ifdef ANALYSIS__STORE_INTEGRATE_INTERVAL_TIMES
    AdaptiveSimpsonHalfIntegrator::s_RecalculateIntegrativeInternalTimes++;
#endif

#ifdef ANALYSIS__CALCULATE_ERROR_BEYOND_LIMIT
    ebl_left.error = internal_left_error_evaluated - scl_error > 0 ? internal_left_error_evaluated - scl_error : 0;
    ebl_left.s = s;
    ebl_left.h = h_2;
#endif
    IntegrateInternalInterval (s + h_2, s + h, scl_error, &values[1], &values[4], &values[5], fright, true, aint_right_S);

#ifdef ANALYSIS__STORE_INTEGRATE_INTERVAL_TIMES
    AdaptiveSimpsonHalfIntegrator::s_RecalculateIntegrativeInternalTimes++;
#endif
  }
  InternalIntegration_SetRightAuxVariables();
#ifdef ANALYSIS__CALCULATE_ERROR_BEYOND_LIMIT
  ebl_right.error = internal_right_error_evaluated - scl_error > 0 ? internal_right_error_evaluated - scl_error : 0;
  ebl_right.s = s + h_2;
  ebl_right.h = h_2;
#endif

  clr[0] = fleft[0];
  clr[1] = fleft[2];
  clr[2] = fleft[4];
  clr[3] = fright[2];
  clr[4] = fright[4];

  InternalIntegration_EndIntegration (s, h_2);

#ifdef ANALYSIS__CALCULATE_ERROR_BEYOND_LIMIT
  m_int_beyonderrorlimit.push_back (ebl_left);
  m_int_beyonderrorlimit.push_back (ebl_right);
#endif

#ifdef ANALYSIS__ACUMULATE_ERROR_BEYOND_LIMIT
  AdaptiveSimpsonHalfIntegrator::s_acumulated_internal_error += ebl_left.error + ebl_right.error;
#endif

  return h;
}

lqc::Vector4d AdaptiveSimpsonHalfIntegrator::ExternalIntegration (lqc::Vector4d C, double p_d, double inner)
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

lqc::Vector4d  aext_left_S, aext_right_S;
bool AdaptiveSimpsonHalfIntegrator::TryExternaIntegration (double s, double h, double tol, double v[], lqc::Vector4d clr[])
{
#ifdef ANALYSIS__STORE_INTEGRATE_INTERVAL_TIMES
  AdaptiveSimpsonHalfIntegrator::s_IntegrativeExternalTimes++;
#endif

  double a = s, c = (s + h) / 2.0, b = s + h;
  double d = (a + c) / 2.0, e = (c + b) / 2.0;

  lqc::Vector4d fa = ExternalIntegration (clr[0], s, 0);
  lqc::Vector4d fd = ExternalIntegration (clr[1], s + (h / 4.0), v[2]);
  lqc::Vector4d fc = ExternalIntegration (clr[2], s + (h / 2.0), v[2] + v[3] + (v[2] + v[3] - v[0]) / 15.0);
  lqc::Vector4d fe = ExternalIntegration (clr[3], s + (3.0 * h / 4.0), (v[2] + v[3] + (v[2] + v[3] - v[0]) / 15.0) + v[4]);
  lqc::Vector4d fb = ExternalIntegration (clr[4], s + h, (v[2] + v[3] + (v[2] + v[3] - v[0]) / 15.0) + (v[4] + v[5] + (v[4] + v[5] - v[1]) / 15.0));

  lqc::Vector4d S = (h / 6.0)*(fa + (4.0 * fc) + fb);
  lqc::Vector4d Sleft = (h / 12.0) * (fa + (4.0 * fd) + fc);
  lqc::Vector4d Sright = (h / 12.0) * (fc + (4.0 * fe) + fb);
  lqc::Vector4d S2 = Sleft + Sright;

  if (ColorErrorEvalFunc (S, S2, tol))
  {
    lqc::Vector4d Sret = S2 + (S2 - S) / 15.0;
    color += Sret;

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
  aext_left_S = Sleft;
  aext_right_S = Sright;
  return false;
}

void AdaptiveSimpsonHalfIntegrator::AdaptiveExternalIntegration (double s, double h, double tol, double* pre_integrated)
{
  minpost = s;
  Cminpost = GetFromTransferFunction (s);
  double d = h;
  double s1 = s + h;
  while (s < s1)
    s += Aux_AdaptiveExternalIntegration (s, s1 - s, (tol * fabs (s1 - s)) / d, pre_integrated);
}

lqc::Vector4d AdaptiveSimpsonHalfIntegrator::ExtenalEvaluation (double p_d, lqc::Vector4d C)
{
  double alpha = ((p_d - minpost) / 6.0) * (Cminpost.w +
    4.0 * GetFromTransferFunction ((minpost + p_d) / 2.0).w + C.w);
  return ExternalIntegration (C, p_d, alpha);
}

bool AdaptiveSimpsonHalfIntegrator::IntegrateExternalInterval (double a, double b, double tol, lqc::Vector4d* pS, lqc::Vector4d clr[], bool force, lqc::Vector4d Spre)
{
#ifdef ANALYSIS__STORE_INTEGRATE_INTERVAL_TIMES
  AdaptiveSimpsonHalfIntegrator::s_IntegrativeExternalTimes++;
#endif
 
  double h = (b - a), c = (a + b) / 2.0,
    d = (a + c) / 2.0, e = (c + b) / 2.0;
  lqc::Vector4d fa = ExtenalEvaluation (a, clr[0]);
  lqc::Vector4d fd = ExtenalEvaluation (d, clr[1]);
  lqc::Vector4d fc = ExtenalEvaluation (c, clr[2]);
  lqc::Vector4d fe = ExtenalEvaluation (e, clr[3]);
  lqc::Vector4d fb = ExtenalEvaluation (b, clr[4]);

  lqc::Vector4d S = Spre;
  lqc::Vector4d Sleft = SimpsonRule<lqc::Vector4d> (fa, fd, fc, h / 2.0);
  lqc::Vector4d Sright = SimpsonRule<lqc::Vector4d> (fc, fe, fb, h / 2.0);
  lqc::Vector4d S2 = Sleft + Sright;

  if (ColorErrorEvalFunc (S, S2, tol) || force)
  {
    (*pS) = S2 + (S2 - S) / 15.0;
    return true;
  }
  aext_left_S = Sleft;
  aext_right_S = Sright;
  return false;
}

double AdaptiveSimpsonHalfIntegrator::Aux_AdaptiveExternalIntegration (double s, double h0, double tol, double* pre_integrated)
{
#ifdef ANALYSIS__CALCULATE_ERROR_BEYOND_LIMIT
  ExternalErrorBeyondLimit ebl_left;
  ExternalErrorBeyondLimit ebl_right;
#endif

  double h = h0;
  double h_2 = h / 2.0;
  double scl_error;

  lqc::Vector4d fleft[5];
  lqc::Vector4d fright[5];
  
  lqc::Vector4d colorleft;
  lqc::Vector4d colorright;

  double a, b, c, d, e;
  a = s; b = s + h_2; c = (a + b) / 2.0;
  fleft[0] = GetFromTransferFunction (a);
  fleft[1] = GetFromTransferFunction (c);
  fleft[2] = GetFromTransferFunction (b);

  while (true)
  {
    scl_error = (tol * h_2) / h0;

    a = s; b = s + h_2; c = (a + b) / 2.0;
    d = (a + c) / 2.0; e = (c + b) / 2.0;
    fleft[4] = fleft[2];
    fleft[2] = fleft[1];
    fleft[1] = GetFromTransferFunction (d);
    fleft[3] = GetFromTransferFunction (e);

    if (IntegrateExternalInterval (s, s + h_2, scl_error, &colorleft, fleft, false, aext_left_S))
    {
      ExternalIntegration_SetLeftAuxVariables();

#ifdef ANALYSIS__CALCULATE_ERROR_BEYOND_LIMIT
      ebl_left.error = external_left_error_evaluated - scl_error > 0 ? external_left_error_evaluated - scl_error : 0;
      ebl_left.s = s;
      ebl_left.h = h_2;
#endif
      break;
    }


#ifdef ANALYSIS__STORE_INTEGRATE_INTERVAL_TIMES
    AdaptiveSimpsonHalfIntegrator::s_RecalculateIntegrativeExternalTimes++;
#endif

    h /= 2.0;
    h_2 = h / 2.0;
  }

  lqc::Vector4d aext_Sleft = aext_left_S;
  lqc::Vector4d aext_Sright = aext_right_S;

  a = s + h_2; b = s + h; c = (a + b) / 2.0;
  d = (a + c) / 2.0; e = (c + b) / 2.0;
  fright[0] = fleft[4];
  fright[1] = GetFromTransferFunction (d);
  fright[2] = GetFromTransferFunction (c);
  fright[3] = GetFromTransferFunction (e);
  fright[4] = GetFromTransferFunction (b);

  if (!IntegrateExternalInterval (s + h_2, s + h, scl_error, &colorright, fright, false, aext_right_S))
  {
    h /= 2.0;
    h_2 = h / 2.0;

    scl_error = (tol * h_2) / h0;

    //arrumar valores da segunda parte
    a = s + h_2; b = s + h; c = (a + b) / 2.0;
    fright[0] = fleft[2];
    fright[1] = GetFromTransferFunction ((a + c) / 2.0);
    fright[2] = fleft[3];
    fright[3] = GetFromTransferFunction ((c + b) / 2.0);
    fright[4] = fleft[4];
    //arrumar valores da primeira parte
    a = s; b = s + h_2; c = (a + b) / 2.0;
    fleft[4] = fleft[2];
    fleft[2] = fleft[1];
    fleft[1] = GetFromTransferFunction ((a + c) / 2.0);
    fleft[3] = GetFromTransferFunction ((c + b) / 2.0);

    IntegrateExternalInterval (s, s + h_2, scl_error, &colorleft, fleft, true, aext_Sleft);
    ExternalIntegration_SetLeftAuxVariables ();
    
#ifdef ANALYSIS__STORE_INTEGRATE_INTERVAL_TIMES
    AdaptiveSimpsonHalfIntegrator::s_RecalculateIntegrativeExternalTimes++;
#endif

#ifdef ANALYSIS__CALCULATE_ERROR_BEYOND_LIMIT
    ebl_left.error = external_left_error_evaluated - scl_error > 0 ? external_left_error_evaluated - scl_error : 0;
    ebl_left.s = s;
    ebl_left.h = h_2;
#endif
    IntegrateExternalInterval (s + h_2, s + h, scl_error, &colorright, fright, true, aext_Sright);

#ifdef ANALYSIS__STORE_INTEGRATE_INTERVAL_TIMES
    AdaptiveSimpsonHalfIntegrator::s_RecalculateIntegrativeExternalTimes++;
#endif
  }
  ExternalIntegration_SetRightAuxVariables();
#ifdef ANALYSIS__CALCULATE_ERROR_BEYOND_LIMIT
  ebl_right.error = external_right_error_evaluated - scl_error > 0 ? external_right_error_evaluated - scl_error : 0;
  ebl_right.s = s + h_2;
  ebl_right.h = h_2;
#endif

  color += colorleft + colorright;
  ExternalIntegration_EndIntegration (s, h_2, h);

#ifdef ANALYSIS__CALCULATE_ERROR_BEYOND_LIMIT
  m_ext_beyonderrorlimit.push_back (ebl_left);
  m_ext_beyonderrorlimit.push_back (ebl_right);
#endif

#ifdef ANALYSIS__ACUMULATE_ERROR_BEYOND_LIMIT
  AdaptiveSimpsonHalfIntegrator::s_acumulated_external_error += ebl_left.error + ebl_right.error;
#endif
  return h;
}

lqc::Vector4d AdaptiveSimpsonHalfIntegrator::GetFromTransferFunction (double p_d)
{
  lqc::Vector4d ret;
  lqc::Vector3d p = minpos + p_d * normalized_step;
  if (!transfer_function || !volume) ret = lqc::Vector4d (0.0);
  else ret = transfer_function->Get (volume_evaluator->GetValueFromVolume (volume, lqc::Vector3f (p.x, p.y, p.z)));
  return ret;
}

void AdaptiveSimpsonHalfIntegrator::Integrate_InitAnalysisAuxVariables ()
{
#ifdef ANALYSIS__ERROR_ALONG_THE_RAY
  error_along_the_ray.m_int_errorintervals.clear ();
  error_along_the_ray.m_ext_errorintervals.clear ();
#endif

  /*TODO MORE*/
}

void AdaptiveSimpsonHalfIntegrator::Integrate_ResetAnalysisAuxVariables ()
{
}

double AdaptiveSimpsonHalfIntegrator::Integrate_CalculateNextInitialInterval (double s, double h, double max_tol, double h0, double s1)
{
  h = h == h0 ? 2.0 * h : h;
  h0 = MIN (h, s1 - s);
#ifdef INTEGRATOR__STEP_CONTROLLED
  h0 = MIN (h0, INTEGRATOR__MAX_STEP_BOUNDARY);
#endif

  return h0;
}

void AdaptiveSimpsonHalfIntegrator::InternalIntegration_SetLeftAuxVariables ()
{
#ifdef ANALYSIS__ERROR_ALONG_THE_RAY
  int_left_error = int_aux_error;
#endif

}

void AdaptiveSimpsonHalfIntegrator::InternalIntegration_SetRightAuxVariables ()
{
#ifdef ANALYSIS__ERROR_ALONG_THE_RAY
  int_right_error = int_aux_error;
#endif
}

void AdaptiveSimpsonHalfIntegrator::InternalIntegration_EndIntegration (double s, double h_2)
{
#ifdef ANALYSIS__ERROR_ALONG_THE_RAY
  error_along_the_ray.m_int_errorintervals.push_back (Analysis_ErrorAlongTheRay::Int_IntervalError (s, h_2, int_left_error));
  error_along_the_ray.m_int_errorintervals.push_back (Analysis_ErrorAlongTheRay::Int_IntervalError (s + h_2, h_2, int_right_error));
#endif
}

void AdaptiveSimpsonHalfIntegrator::ExternalIntegration_SetLeftAuxVariables ()
{
#ifdef ANALYSIS__ERROR_ALONG_THE_RAY
  ext_left_error = ext_aux_error;
#endif
}

void AdaptiveSimpsonHalfIntegrator::ExternalIntegration_SetRightAuxVariables ()
{
#ifdef ANALYSIS__ERROR_ALONG_THE_RAY
  ext_right_error = ext_aux_error;
#endif
}

void AdaptiveSimpsonHalfIntegrator::ExternalIntegration_EndIntegration (double s, double h_2, double h)
{
#ifdef ANALYSIS__ERROR_ALONG_THE_RAY
  error_along_the_ray.m_ext_errorintervals.push_back (Analysis_ErrorAlongTheRay::Ext_IntervalError (s, h_2, ext_left_error));
  error_along_the_ray.m_ext_errorintervals.push_back (Analysis_ErrorAlongTheRay::Ext_IntervalError (s + h_2, h_2, ext_right_error));
#endif

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
  ExternalCumulativeError exce;
  exce.s = s + h;
  exce.h = h;
  exce.err = color;
  m_excumulative_error.push_back (exce);
#endif
}