#include "SimpsonErrorHalfProjectionIntegrator.h"

#include "VolumeEvaluator.h"

#include <iostream>
#include <fstream>
#include <cmath>
#include <cerrno>
#include <cfenv>
#include <cstring>

//http://martin.ankerl.com/2012/01/25/optimized-approximative-pow-in-c-and-cpp/

///Reutilizar não apenas amostras, mas também avaliações de intervalos
int SimpsonErrorHalfProjectionIntegrator::s_IntegrativeExternalTimes = 0;
int SimpsonErrorHalfProjectionIntegrator::s_IntegrativeInternalTimes = 0;

int SimpsonErrorHalfProjectionIntegrator::s_RecalculateIntegrativeExternalTimes = 0;
int SimpsonErrorHalfProjectionIntegrator::s_RecalculateIntegrativeInternalTimes = 0;

bool SimpsonErrorHalfProjectionIntegrator::ColorErrorEvalFunc (lqc::Vector4d a, lqc::Vector4d b, double tol)
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

SimpsonErrorHalfProjectionIntegrator::SimpsonErrorHalfProjectionIntegrator (VolumeEvaluator* veva)
{
  volume_evaluator = veva;
  m_integrate_method = 1;
}

SimpsonErrorHalfProjectionIntegrator::~SimpsonErrorHalfProjectionIntegrator ()
{
  Reset ();
}

void SimpsonErrorHalfProjectionIntegrator::Reset ()
{
  m_clc_proj_newinternalprojection = 0;
  m_clc_proj_internal_error = 0;
  m_clc_proj_external_error = lqc::Vector4d (0);
}

void SimpsonErrorHalfProjectionIntegrator::Init (lqc::Vector3d minp, lqc::Vector3d maxp, vr::Volume* vol, vr::TransferFunction* tf)
{
  volume = vol;
  transfer_function = tf;

  minpos = minp;
  maxpos = maxp;
  normalized_step = lqc::Vector3d::Normalize (maxpos - minpos);

  pre_integrated = 0.0;
  color = lqc::Vector4d (0);
}

void SimpsonErrorHalfProjectionIntegrator::Integrate (double s0, double s1, double tol, double h0)
{
  double exponent = (1.0 / 3.0);
  double tol_int = tol, tol_ext = tol;
  double h = std::min (h0, s1 - s0), hproj = 0;
  double s = s0;
  Cminpost = GetFromTransferFunction (s);
  while (s < s1)
  {
    minpost = s;

    h = std::max (h, hproj);
    h = std::max (h, REFERENCE_MIN_STEP);
    h = std::min (h, s1 - s);
    h = std::min (h, REFERENCE_MAX_STEP);

    //Calcular integral interna
    h = InternalIntegral (s, h, (tol_int * h) / (s1 - s0), &hproj);
    h = std::min (h, hproj);
    h = std::max (h, std::min (REFERENCE_MIN_STEP, s1 - s));
    //Calcular integral externa
    h = ExternalIntegralOneStep (s, h, (tol_ext * h) / (s1 - s0));
    //ExternalIntegral (s, h, (tol_ext * h) / (s1 - s0));

    //Increment evaluation
    s = s + h;
  }
}

double SimpsonErrorHalfProjectionIntegrator::InternalIntegral (double s, double h0, double error, double* hproj)
{
  double h = h0;
  double h_error = error;
  double exponent = (1.0 / 3.0);

  double ierror = IntegrateInternalInterval (s, h, h_error);
  (*hproj) = std::min (std::pow (((15.0*h_error) / ierror), exponent) * h, REFERENCE_MAX_STEP);

  bool done = ierror <= 15.0*h_error;
  while (!done && (*hproj) < h && h > REFERENCE_MIN_STEP)
  {
    h = (*hproj);
    h_error = (error * h) / h0;
    ierror = IntegrateInternalInterval (s, h, h_error);
    (*hproj) = std::min (std::pow (((15.0*h_error) / ierror), exponent) * h, REFERENCE_MAX_STEP);
    (*hproj) = std::min ((*hproj), h / 2.0);

    done = ierror <= 15.0*h_error;
  }

  return h;
}

double SimpsonErrorHalfProjectionIntegrator::ExternalIntegral (double s, double h0, double error)
{
  double h = h0;
  double s1 = s + h;
  double H_error = error;
  double exponent = (1.0 / 3.0);

  lqc::Vector4d S = lqc::Vector4d (0);
  lqc::Vector4d gftd_b = lqc::Vector4d (0);

  while (s < s1)
  {
    h = s1 - s;
    H_error = (error * h) / h0;
    if (!IntegrateExternalInterval (s, h, H_error, &pre_integrated, h <= REFERENCE_MIN_STEP))
    {
      h = std::max (std::pow (((15.0*H_error) / MaxExternalError (external_error_aux)), exponent) * h, REFERENCE_MIN_STEP);

      double a, c, b;
      a = s; b = s + h; c = (a + b) / 2.0;
      lqc::Vector4d gftd_c = GetFromTransferFunction (c);
      gftd_b = GetFromTransferFunction (b);

      lqc::Vector4d fa = ExtenalEvaluation (a, Cminpost);
      lqc::Vector4d fc = ExtenalEvaluation (c, gftd_c);
      lqc::Vector4d fb = ExtenalEvaluationMiddle (b, gftd_b, gftd_c, &pre_integrated);

      S = SimpsonRule<lqc::Vector4d> (fa, fc, fb, h);

      color += S;
      Cminpost = gftd_b;
    }
    
    s = s + h;
    minpost = s;
  }

  return h;
}

double SimpsonErrorHalfProjectionIntegrator::InternalIntegralOneStep (double s, double h0, double error, double* hproj)
{
  double h = h0;
  double h_error = error;
  double exponent = (1.0 / 3.0);

  double ierror = IntegrateInternalInterval (s, h, h_error);
  (*hproj) = std::min (RESHAPE_TRUTH_FACTOR*std::pow (((15.0*h_error) / ierror), exponent) * h, REFERENCE_MAX_STEP);

  return h;
}

double SimpsonErrorHalfProjectionIntegrator::ExternalIntegralOneStep (double s, double h0, double error)
{
  double h = h0;
  double H_error = error;
  double exponent = (1.0 / 3.0);

  while (!IntegrateExternalInterval (s, h, H_error, &pre_integrated, h <= REFERENCE_MIN_STEP))
  {
    double exhproj = std::max (std::pow (((15.0*H_error) / MaxExternalError (external_error_aux)), exponent) * h, REFERENCE_MIN_STEP);
    h = std::min (exhproj, h / 2.0);
    H_error = (error * h) / h0;

    if (h <= REFERENCE_MIN_STEP)
    {
      double a, c, b;
      a = s; b = s + h; c = (a + b) / 2.0;
      lqc::Vector4d gftd_c = GetFromTransferFunction (c);
      lqc::Vector4d gftd_b = GetFromTransferFunction (b);

      lqc::Vector4d fa = ExtenalEvaluation (a, Cminpost);

      lqc::Vector4d fc = ExtenalEvaluation (c, gftd_c);
      //lqc::Vector4d fc = ExtenalEvaluationMiddle (c, gftd_c, GetFromTransferFunction ((a + c) / 2.0), &pre_integrated);
      //minpost = c; Cminpost = gftd_c;

      //lqc::Vector4d fd = ExtenalEvaluation ((a + c) / 2.0, GetFromTransferFunction ((a + c) / 2.0));
      //lqc::Vector4d fe = ExtenalEvaluation ((c + b) / 2.0, GetFromTransferFunction ((c + b) / 2.0));

      lqc::Vector4d fb = ExtenalEvaluationMiddle (b, gftd_b, gftd_c, &pre_integrated);
      //lqc::Vector4d fb = ExtenalEvaluationMiddle (b, gftd_b, GetFromTransferFunction ((c + b) / 2.0), &pre_integrated);
      //minpost = b; Cminpost = gftd_b;

      lqc::Vector4d S = SimpsonRule<lqc::Vector4d> (fa, fc, fb, h);
      color += S;
      //lqc::Vector4d Sleft = SimpsonRule<lqc::Vector4d> (fa, fd, fc, h / 2.0);
      //lqc::Vector4d Sright = SimpsonRule<lqc::Vector4d> (fc, fe, fb, h / 2.0);
      //color += (Sleft + Sright) + ((Sleft + Sright) - S) / 15.0;

      Cminpost = gftd_b;
      break;
    }



    /*
    double a, c, b;
    a = s; b = s + h; c = (a + b) / 2.0;
    lqc::Vector4d gftd_c = GetFromTransferFunction (c);
    lqc::Vector4d gftd_b = GetFromTransferFunction (b);

    lqc::Vector4d fa = ExtenalEvaluation (a, Cminpost);

    lqc::Vector4d fc = ExtenalEvaluation (c, gftd_c);
    //lqc::Vector4d fc = ExtenalEvaluationMiddle (c, gftd_c, GetFromTransferFunction ((a + c) / 2.0), &pre_integrated);
    //minpost = c; Cminpost = gftd_c;

    //lqc::Vector4d fd = ExtenalEvaluation ((a + c) / 2.0, GetFromTransferFunction ((a + c) / 2.0));
    //lqc::Vector4d fe = ExtenalEvaluation ((c + b) / 2.0, GetFromTransferFunction ((c + b) / 2.0));

    lqc::Vector4d fb = ExtenalEvaluationMiddle (b, gftd_b, gftd_c, &pre_integrated);
    //lqc::Vector4d fb = ExtenalEvaluationMiddle (b, gftd_b, GetFromTransferFunction ((c + b) / 2.0), &pre_integrated);
    //minpost = b; Cminpost = gftd_b;

    lqc::Vector4d S = SimpsonRule<lqc::Vector4d> (fa, fc, fb, h);
    color += S;
    //lqc::Vector4d Sleft = SimpsonRule<lqc::Vector4d> (fa, fd, fc, h / 2.0);
    //lqc::Vector4d Sright = SimpsonRule<lqc::Vector4d> (fc, fe, fb, h / 2.0);
    //color += (Sleft + Sright) + ((Sleft + Sright) - S) / 15.0;

    Cminpost = gftd_b;*/
  }
  //m_clc_proj_internal_error += CalculateInternalError (s, h, (tol_int * h) / (s1 - s0));

  return h;
}

double SimpsonErrorHalfProjectionIntegrator::IntegrateInternalInterval (double s, double h, double tol)
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

lqc::Vector4d SimpsonErrorHalfProjectionIntegrator::ExtenalEvaluation (double p_d, lqc::Vector4d C, double* pre_alpha)
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

lqc::Vector4d SimpsonErrorHalfProjectionIntegrator::ExtenalEvaluationMiddle (double p_d, lqc::Vector4d C, lqc::Vector4d Cmid, double* pre_alpha)
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

bool SimpsonErrorHalfProjectionIntegrator::IntegrateExternalInterval (double s, double h, double tol, double* pre_alpha, bool minorstep)
{
  double a, d, c, e, b;
  a = s; b = s + h; c = (a + b) / 2.0;
  d = (a + c) / 2.0; e = (c + b) / 2.0;

  double pre_a;
  lqc::Vector4d gf_b = GetFromTransferFunction (b);
  lqc::Vector4d gf_d = GetFromTransferFunction (d);
  lqc::Vector4d gf_c = GetFromTransferFunction (c);

  lqc::Vector4d fa = ExtenalEvaluation (a, Cminpost);
  lqc::Vector4d fd = ExtenalEvaluation (d, gf_d);
  lqc::Vector4d fc = ExtenalEvaluationMiddle (c, gf_c, gf_d);
  lqc::Vector4d fe = ExtenalEvaluation (e, GetFromTransferFunction (e));
  lqc::Vector4d fb = ExtenalEvaluationMiddle (b, gf_b, gf_c, &pre_a);

  lqc::Vector4d S = SimpsonRule<lqc::Vector4d> (fa, fc, fb, h);
  lqc::Vector4d Sleft = SimpsonRule<lqc::Vector4d> (fa, fd, fc, h / 2.0);
  lqc::Vector4d Sright = SimpsonRule<lqc::Vector4d> (fc, fe, fb, h / 2.0);
  lqc::Vector4d S2 = Sleft + Sright;

  if (ColorErrorEvalFunc (S, S2, tol) || minorstep)
  {
    color += S2 + (S2 - S) / 15.0;
    (*pre_alpha) = pre_a;
    Cminpost = gf_b;
    return true;
  }
  return false;
}

lqc::Vector4d SimpsonErrorHalfProjectionIntegrator::GetFromTransferFunction (double p_d)
{
  lqc::Vector4d ret;
  lqc::Vector3d p = minpos + p_d * normalized_step;
  if (!transfer_function || !volume) ret = lqc::Vector4d (0.0);
  else ret = transfer_function->Get (volume_evaluator->GetValueFromVolume (volume, lqc::Vector3f (p.x, p.y, p.z)));
  return ret;
}