#include "SimpsonIEHPEHIntegrator.h"

#include "VolumeEvaluator.h"

#include <iostream>
#include <fstream>
#include <cmath>
#include <cerrno>
#include <cfenv>
#include <cstring>

//http://martin.ankerl.com/2012/01/25/optimized-approximative-pow-in-c-and-cpp/

///Reutilizar não apenas amostras, mas também avaliações de intervalos
int AdaptiveSimpsonIEHPEHIntegrator::s_IntegrativeExternalTimes = 0;
int AdaptiveSimpsonIEHPEHIntegrator::s_IntegrativeInternalTimes = 0;

int AdaptiveSimpsonIEHPEHIntegrator::s_RecalculateIntegrativeExternalTimes = 0;
int AdaptiveSimpsonIEHPEHIntegrator::s_RecalculateIntegrativeInternalTimes = 0;

bool AdaptiveSimpsonIEHPEHIntegrator::ColorErrorEvalFunc (lqc::Vector4d a, lqc::Vector4d b, double tol)
{
  tol = 15.0 * tol;

  double error_r = fabs (a.x - b.x);
  double error_g = fabs (a.y - b.y);
  double error_b = fabs (a.z - b.z);
  double error_a = fabs (a.w - b.w);

  external_error_aux = lqc::Vector4d (error_r, error_g, error_b, error_a);

  return (tol >= error_r && tol >= error_g
    && tol >= error_b && tol >= error_a);
}

AdaptiveSimpsonIEHPEHIntegrator::AdaptiveSimpsonIEHPEHIntegrator (VolumeEvaluator* veva)
{
  volume_evaluator = veva;
  m_integrate_method = 1;
}

AdaptiveSimpsonIEHPEHIntegrator::~AdaptiveSimpsonIEHPEHIntegrator ()
{
  Reset ();
}

void AdaptiveSimpsonIEHPEHIntegrator::Reset ()
{
  m_clc_proj_newinternalprojection = 0;
  m_clc_proj_internal_error = 0;
  m_clc_proj_external_error = lqc::Vector4d (0);
}

void AdaptiveSimpsonIEHPEHIntegrator::Init (lqc::Vector3d minp, lqc::Vector3d maxp, vr::Volume* vol, vr::TransferFunction* tf)
{
  volume = vol;
  transfer_function = tf;

  minpos = minp;
  maxpos = maxp;
  normalized_step = lqc::Vector3d::Normalize (maxpos - minpos);

  pre_integrated = 0.0;
  color = lqc::Vector4d (0);
}

double neg = false;
void AdaptiveSimpsonIEHPEHIntegrator::Integrate (double s0, double s1, double tol, double h0)
{
  double exponent = (1.0 / 3.0);
  double tol_int = tol, tol_ext = tol;
  double h = std::min (h0, s1 - s0), hproj = 0;
  double s = s0;
  Cminpost = GetFromTransferFunction (s);
  minpost = s;
  while (s1 > s)
  {
    if (!neg)
      h = std::max (h, hproj);
    //h = std::min (h, 10.0);
    h = std::max (h, std::min (REFERENCE_MIN_STEP, s1 - s));

    //Calcular integral interna
    h = InternalIntegral (s, h, (tol_int * h) / (s1 - s0), &hproj);
    h = std::max (h, std::min (REFERENCE_MIN_STEP, s1 - s));

    //Calcular integral externa
    ExternalIntegral (s, h, (tol_ext * h) / (s1 - s0));
    
    //Increment evaluation
    s = s + h;
  }
}

//Se o passo for negado, não aumentar o passo para a próxima iteração
double AdaptiveSimpsonIEHPEHIntegrator::InternalIntegral (double s, double h0, double error, double* hproj)
{
  double h = h0;
  double h_error = error;
  double ierror = IntegrateInternalInterval (s, h, h_error);
  
  (*hproj) = std::min (RESHAPE_TRUTH_FACTOR*std::pow (((15.0*h_error) / ierror), (1.0 / 3.0)) * h, 2.0*h);

  bool done = ierror <= 15.0*h_error;
  neg = false;
  
  while (!done && h >= REFERENCE_MIN_STEP)
  {
    h = (*hproj);
    h_error = (error * h) / h0;
    ierror = IntegrateInternalInterval (s, h, h_error);

    (*hproj) = std::min (RESHAPE_TRUTH_FACTOR*std::pow (((15.0*h_error) / ierror), (1.0 / 3.0)) * h, 2.0*h);
    
    done = ierror <= 15.0*h_error;
    neg = true;
  }

  return h;
}

void AdaptiveSimpsonIEHPEHIntegrator::ExternalIntegral (double s, double h0, double error)
{
  double s1 = s + h0;
  lastValue = GetFromTransferFunction (s1);
  while (s1 > s)
    s += AdaptiveExternalIntegration (s, s1 - s, (error * (s1 - s) / h0));
}

double AdaptiveSimpsonIEHPEHIntegrator::AdaptiveExternalIntegration (double s, double h, double error)
{
  lqc::Vector4d color_left (0);
  double pre_alpha;

  f_left[0] = Cminpost;
  f_left[1] = GetFromTransferFunction (s + (h / 4.0));
  f_left[2] = GetFromTransferFunction (s + (h / 2.0));
  f_left[3] = GetFromTransferFunction (s + (3.0*h / 4.0));
  f_left[4] = lastValue;

  if (IntegrateExternalInterval (s, h, error, &pre_alpha, h <= REFERENCE_MIN_STEP, &color_left, f_left))
  {
    color += color_left;

    pre_integrated = pre_alpha;
    minpost = s + h;
    Cminpost = f_left[4];

    return h;
  }

  double h0 = h;
  double h_2 = h / 2.0;
  double scl_error;

  while (true)
  {
    scl_error = (error * h_2) / h0;

    f_left[4] = f_left[2];
    f_left[2] = f_left[1];
    f_left[1] = GetFromTransferFunction (s + h_2 / 4.0);
    f_left[3] = GetFromTransferFunction (s + (3.0*h_2 / 4.0));

    if (IntegrateExternalInterval (s, h_2, scl_error, NULL, h_2 <= REFERENCE_MIN_STEP, &color_left, f_left))
      break;

    h /= 2.0;
    h_2 = h / 2.0;
  }

  f_right[0] = f_left[4];
  f_right[1] = GetFromTransferFunction (s + h_2 + (h_2 / 4.0));
  f_right[2] = GetFromTransferFunction (s + h_2 + (h_2 / 2.0));
  f_right[3] = GetFromTransferFunction (s + h_2 + (3.0*h_2 / 4.0));
  f_right[4] = GetFromTransferFunction (s + h);

  lqc::Vector4d color_right (0);
  if (!IntegrateExternalInterval (s + h_2, h_2, scl_error, &pre_alpha, h_2 <= REFERENCE_MIN_STEP, &color_right, f_right))
  {
    h /= 2.0;
    h_2 = h / 2.0;

    scl_error = (error * h_2) / h0;

    /////////////////////////////////////////////
    //f_right[4] = f_left[4];
    //
    //lqc::Vector4d flr = ExtenalEvaluationMiddle (s + h_2, f_left[2], f_left[1]);
    //color_left = SimpsonRule<lqc::Vector4d> (
    //  ExtenalEvaluation (s            , f_left[0]),
    //  ExtenalEvaluation (s + (h_2/2.0), f_left[1]),
    //  flr, h_2);
    //
    //color_right = SimpsonRule<lqc::Vector4d> (flr,
    //  ExtenalEvaluation (s + h_2 + (h_2 / 2.0), f_left[3]),
    //  ExtenalEvaluationMiddle (s + h, f_left[4], f_left[2], &pre_alpha),
    //  h_2);
    /////////////////////////////////////////////
    f_right[0] = f_left[2];
    f_right[1] = GetFromTransferFunction (s + h_2 + (h_2 / 4.0));
    f_right[2] = f_left[3];
    f_right[3] = GetFromTransferFunction (s + h_2 + (3.0*h_2 / 4.0));
    f_right[4] = f_left[4];
    //arrumar valores da primeira parte
    f_left[4] = f_left[2];
    f_left[2] = f_left[1];
    f_left[1] = GetFromTransferFunction (s + h_2 / 4.0);
    f_left[3] = GetFromTransferFunction (s + (3.0*h_2 / 4.0));
    
    IntegrateExternalInterval (s      , h_2, scl_error, NULL      , true, &color_left , f_left);
    IntegrateExternalInterval (s + h_2, h_2, scl_error, &pre_alpha, true, &color_right, f_right);
    /////////////////////////////////////////////
  }
  color += color_left + color_right;

  pre_integrated = pre_alpha;
  minpost = s + h;
  Cminpost = f_right[4];

  return h;
}

double AdaptiveSimpsonIEHPEHIntegrator::IntegrateInternalInterval (double s, double h, double tol)
{
  double d, c, e, b;
  b = s + h; c = (s + b) / 2.0;
  d = (s + c) / 2.0; e = (c + b) / 2.0;

  double fa = Cminpost.w;
  double fd = GetFromTransferFunction(d).w;
  double fc = GetFromTransferFunction(c).w;
  double fe = GetFromTransferFunction(e).w;
  double fb = GetFromTransferFunction(b).w;

  double S = SimpsonRule<double> (fa, fc, fb, h);
  double Sleft = SimpsonRule<double> (fa, fd, fc, h / 2.0); 
  double Sright = SimpsonRule<double> (fc, fe, fb, h / 2.0);

  return fabs ((Sleft + Sright) - S);
}

lqc::Vector4d AdaptiveSimpsonIEHPEHIntegrator::ExtenalEvaluation (double p_d, lqc::Vector4d C, double* pre_alpha)
{
  double alpha = 0.0;
  double h = p_d - minpost;
  if (h != 0)
    alpha = (h / 6.0) * (Cminpost.w + 4.0 * GetFromTransferFunction ((minpost + p_d) / 2.0).w + C.w);
  
  double innerint = exp (-(pre_integrated + alpha));

  if (pre_alpha)
    (*pre_alpha) = pre_integrated + alpha;

  double alphachannel = C.w*innerint;
  return lqc::Vector4d (
    alphachannel * C.x,
    alphachannel * C.y,
    alphachannel * C.z,
    alphachannel
    );
}

lqc::Vector4d AdaptiveSimpsonIEHPEHIntegrator::ExtenalEvaluationMiddle (double p_d, lqc::Vector4d C, lqc::Vector4d Cmid, double* pre_alpha)
{
  double alpha = 0.0;
  double h = p_d - minpost;
  if (h != 0)
    alpha = (h / 6.0) * (Cminpost.w + 4.0 * Cmid.w + C.w);

  double innerint = exp (-(pre_integrated + alpha));

  if (pre_alpha)
    (*pre_alpha) = pre_integrated + alpha;

  double alphachannel = C.w*innerint;
  return lqc::Vector4d (
    alphachannel * C.x,
    alphachannel * C.y,
    alphachannel * C.z,
    alphachannel
    );
}

bool AdaptiveSimpsonIEHPEHIntegrator::IntegrateExternalInterval (double s, double h, double tol, double* pre_alpha, bool minorstep, lqc::Vector4d* color, lqc::Vector4d f[])
{
  double a, d, c, e, b;
  a = s; b = s + h; c = (a + b) / 2.0;
  d = (a + c) / 2.0; e = (c + b) / 2.0;

  double pre_a;
  
  lqc::Vector4d fa = ExtenalEvaluation (a, f[0]);
  lqc::Vector4d fd = ExtenalEvaluation (d, f[1]);
  lqc::Vector4d fc = ExtenalEvaluation (c, f[2]);
  lqc::Vector4d fe = ExtenalEvaluation (e, f[3]);
  lqc::Vector4d fb = ExtenalEvaluation (b, f[4], &pre_a);

  lqc::Vector4d S = SimpsonRule<lqc::Vector4d> (fa, fc, fb, h);
  lqc::Vector4d Sleft = SimpsonRule<lqc::Vector4d> (fa, fd, fc, h / 2.0);
  lqc::Vector4d Sright = SimpsonRule<lqc::Vector4d> (fc, fe, fb, h / 2.0);
  lqc::Vector4d S2 = Sleft + Sright;

  if (ColorErrorEvalFunc (S, S2, tol) || minorstep)
  {
    (*color) = S2 + (S2 - S) / 15.0;
    if (pre_alpha)
    (*pre_alpha) = pre_a;
    return true;
  }
  return false;
}

lqc::Vector4d AdaptiveSimpsonIEHPEHIntegrator::GetFromTransferFunction (double p_d)
{
  lqc::Vector4d ret;
  lqc::Vector3d p = minpos + p_d * normalized_step;
  if (!transfer_function || !volume) ret = lqc::Vector4d (0.0);
  else ret = transfer_function->Get (volume_evaluator->GetValueFromVolume (volume, lqc::Vector3f (p.x, p.y, p.z)));
  return ret;
}

bool ex_err = false;
void AdaptiveSimpsonIEHPEHIntegrator::Test_ExternalIntegral (double s, double h0, double error)
{
  double s1 = s + h0;
  double h = s1 - s;
  lastValue = GetFromTransferFunction (s1);
  while (s1 > s)
  {
    h = std::min (h, s1 - s);
    ex_err = false;
    h = Test_AdaptiveExternalIntegration (s, h, (error * h / h0));
    s = s + h;
    if (!ex_err)
      h = 2.0*h;
  }
}

double AdaptiveSimpsonIEHPEHIntegrator::Test_AdaptiveExternalIntegration (double s, double h, double error)
{
  lqc::Vector4d color_left (0);
  double pre_alpha;

  f_left[0] = Cminpost;
  f_left[1] = GetFromTransferFunction (s + (h / 4.0));
  f_left[2] = GetFromTransferFunction (s + (h / 2.0));
  f_left[3] = GetFromTransferFunction (s + (3.0*h / 4.0));
  f_left[4] = GetFromTransferFunction (s + h);

  if (!IntegrateExternalInterval (s, h, error, &pre_alpha, h <= REFERENCE_MIN_STEP, &color_left, f_left))
  {
    ex_err = true;

    double h0 = h;
    double scl_error;

    h /= 2.0;

    while (true)
    {
      scl_error = (error * h) / h0;

      f_left[4] = f_left[2];
      f_left[2] = f_left[1];
      f_left[1] = GetFromTransferFunction (s + h / 4.0);
      f_left[3] = GetFromTransferFunction (s + (3.0*h / 4.0));

      if (IntegrateExternalInterval (s, h, scl_error, &pre_alpha, h <= REFERENCE_MIN_STEP, &color_left, f_left))
        break;

      h /= 2.0;
    }
  }
  color += color_left;

  pre_integrated = pre_alpha;
  minpost = s + h;
  Cminpost = f_left[4];

  return h;
}