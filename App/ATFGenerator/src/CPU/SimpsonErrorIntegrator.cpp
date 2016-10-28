#include "SimpsonErrorIntegrator.h"

#include "VolumeEvaluator.h"

#include <iostream>
#include <fstream>
#include <cmath>
#include <cerrno>
#include <cfenv>
#include <cstring>

//http://martin.ankerl.com/2012/01/25/optimized-approximative-pow-in-c-and-cpp/

///Reutilizar não apenas amostras, mas também avaliações de intervalos
int SimpsonErrorIntegrator::s_IntegrativeExternalTimes = 0;
int SimpsonErrorIntegrator::s_IntegrativeInternalTimes = 0;

int SimpsonErrorIntegrator::s_RecalculateIntegrativeExternalTimes = 0;
int SimpsonErrorIntegrator::s_RecalculateIntegrativeInternalTimes = 0;

bool SimpsonErrorIntegrator::ColorErrorEvalFunc (lqc::Vector4d a, lqc::Vector4d b, double tol)
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

SimpsonErrorIntegrator::SimpsonErrorIntegrator (VolumeEvaluator* veva)
{
  volume_evaluator = veva;
  m_integrate_method = 1;
}

SimpsonErrorIntegrator::~SimpsonErrorIntegrator ()
{
  Reset ();
}

void SimpsonErrorIntegrator::Reset ()
{
  m_clc_proj_newinternalprojection = 0;
  m_clc_proj_internal_error = 0;
  m_clc_proj_external_error = lqc::Vector4d (0);
}

void SimpsonErrorIntegrator::Init(lqc::Vector3d minp, lqc::Vector3d maxp, vr::Volume* vol, vr::TransferFunction* tf)
{
  volume = vol;
  transfer_function = tf;

  minpos = minp;
  maxpos = maxp;
  normalized_step = lqc::Vector3d::Normalize (maxpos - minpos);

  pre_integrated = 0.0;
  color = lqc::Vector4d (0);
}

void SimpsonErrorIntegrator::Integrate (double s0, double s1, double tol, double h0)
{
  IntegrateError (s0, s1, tol, h0);
}

double eierrorie, eiemaxie, ehie, ehprojie;
double eifa, eifd, eifc, eife, eifb;
void SimpsonErrorIntegrator::IntegrateError (double s0, double s1, double tol, double h0)
{
  double tol_int = tol, tol_ext = tol;
  double h = std::min (h0, s1 - s0);
  double s = s0;
  Cminpost = GetFromTransferFunction (s);
  while (s < s1)
  {
    minpost = s;

    //Calcular integral interna
    double h_error = (tol_int * h) / (s1 - s0);
    h = std::min (std::pow (((15.0*h_error) / IntegrateInternalInterval (s, h, h_error)), 0.25) * h, 2.0 * h);
    h = std::min (h, s1 - s);
    //h = std::min (h, 10.0);
    //m_clc_proj_internal_error += CalculateInternalError (s, h, (tol_int * h) / (s1 - s0));

    //Calcular integral externa
    double H_error = (tol_ext * h) / (s1 - s0);
    if (!IntegrateExternalInterval (s, h, H_error, &pre_integrated, false))
    {
      h = std::pow (((15.0*H_error) / MaxExternalError (external_error_aux)), 0.25) * h;

      double a, c, b;
      a = s; b = s + h; c = (a + b) / 2.0;
      lqc::Vector4d gftd_c = GetFromTransferFunction (c);
      lqc::Vector4d gftd_b = GetFromTransferFunction (b);

      lqc::Vector4d fa = ExtenalEvaluation (a, Cminpost);
      lqc::Vector4d fc = ExtenalEvaluation (c, gftd_c);
      //lqc::Vector4d fd = ExtenalEvaluation ((a + c) / 2.0, GetFromTransferFunction ((a + c) / 2.0));
      //lqc::Vector4d fe = ExtenalEvaluation ((c + b) / 2.0, GetFromTransferFunction ((c + b) / 2.0));
      lqc::Vector4d fb = ExtenalEvaluationMiddle (b, gftd_b, gftd_c, &pre_integrated);

      lqc::Vector4d S = SimpsonRule<lqc::Vector4d> (fa, fc, fb, h);
      color += S;
      //lqc::Vector4d Sleft = SimpsonRule<lqc::Vector4d> (fa, fd, fc, h / 2.0);
      //lqc::Vector4d Sright = SimpsonRule<lqc::Vector4d> (fc, fe, fb, h / 2.0);
      //color += (Sleft + Sright) + ((Sleft + Sright) - S) / 15.0;

      Cminpost = gftd_b;
    }
    //Increment evaluation
    s = s + h;
  }
}

double SimpsonErrorIntegrator::IntegrateInternalInterval (double s, double h, double tol)
{
  double d, c, e, b;
  b = s + h; c = (s + b) / 2.0;
  d = (s + c) / 2.0; e = (c + b) / 2.0;

  double fa = Cminpost.w;
  double fd = GetFromTransferFunction(d).w;
  double fc = GetFromTransferFunction(c).w;
  double fe = GetFromTransferFunction(e).w;
  double fb = GetFromTransferFunction(b).w;

  eifa = fa; eifd = fd; eifc = fc; eife = fe; eifb = fb;

  double S = SimpsonRule<double> (fa, fc, fb, h);
  double Sleft = SimpsonRule<double> (fa, fd, fc, h / 2.0); 
  double Sright = SimpsonRule<double> (fc, fe, fb, h / 2.0);

  return fabs ((Sleft + Sright) - S);
}

double SimpsonErrorIntegrator::CalculateInternalError (double s, double h, double tol, bool higher)
{
  double fa;
  double fd;
  double fc;
  double fe;
  double fb;
  double ret = 0.0;
  if (h > 0.01)
  {
    double d, c, e, b;
    b = s + h; c = (s + b) / 2.0;
    d = (s + c) / 2.0; e = (c + b) / 2.0;

    fa = GetFromTransferFunction (s).w;
    fd = GetFromTransferFunction (d).w;
    fc = GetFromTransferFunction (c).w;
    fe = GetFromTransferFunction (e).w;
    fb = GetFromTransferFunction (b).w;

    double S = SimpsonRule<double> (fa, fc, fb, h);
    double Sleft = SimpsonRule<double> (fa, fd, fc, h / 2.0);
    double Sright = SimpsonRule<double> (fc, fe, fb, h / 2.0);

    ret = fabs ((Sleft + Sright - S));


    //std::fstream state_file2;
    //state_file2.open ("hie2.txt", std::ios::in | std::ios::out | std::ios::app);
    //if (ret == 0.0) state_file2 << 0 << "\n";
    //else state_file2 << fabs (((tol*15.0) - ret) / ret) << "\n";
    //state_file2.close ();
  }
  else
  {
    //std::fstream state_file2;
    //state_file2.open ("hie2.txt", std::ios::in | std::ios::out | std::ios::app);
    //state_file2 << 0.0 << "\n";
    //state_file2.close ();
  }
  //printf ("Proportional error: %lf\n", ret / (tol*15.0));.
  //std::fstream state_file;
  //state_file.open ("hie.txt", std::ios::in | std::ios::out | std::ios::app);
  //state_file << ret / (tol*15.0) << "\n";
  if (ret / (tol*15.0) > 10)
  {


    printf ("Passo: %lf -> %lf\n", ehie, ehprojie);
    printf ("Erro Anterior: %lf, Erro Max: %lf\n", eierrorie, eiemaxie);
    printf ("Novo Erro    : %lf, Erro Max: %lf\n", ret, (tol*15.0));
    printf ("Amostras Anteriores: %lf %lf %lf %lf %lf\n", eifa, eifd, eifc, eife, eifb);
    printf ("Novas Amostras     : %lf %lf %lf %lf %lf\n", fa,fd,fc,fe,fb);
    printf ("Quanto seria o novo passo: %lf\n", std::pow (((tol*15.0) / ret), (1.0/3.0)) * h);
    printf ("------------------------------------\n");

    getchar ();
  }
  //state_file.close ();


  return ret;
}

lqc::Vector4d SimpsonErrorIntegrator::CalculateExternalError (double s, double h, double tol)
{
  lqc::Vector4d err (0);
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

  if (!ColorErrorEvalFunc (S, S2, tol))
  {
    tol = 15.0 * tol;

    double error_r = fabs (S.x - S2.x);
    double error_g = fabs (S.y - S2.y);
    double error_b = fabs (S.z - S2.z);
    double error_a = fabs (S.w - S2.w);

    if (error_r > tol)
      err.x = error_r - tol;
    if (error_g > tol)
      err.y = error_g - tol;
    if (error_b > tol)
      err.z = error_b - tol;
    if (error_a > tol)
      err.w = error_a - tol;
  }
  return err;
}

lqc::Vector4d SimpsonErrorIntegrator::ExtenalEvaluation (double p_d, lqc::Vector4d C, double* pre_alpha)
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

lqc::Vector4d SimpsonErrorIntegrator::ExtenalEvaluationMiddle (double p_d, lqc::Vector4d C, lqc::Vector4d Cmid, double* pre_alpha)
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

bool SimpsonErrorIntegrator::IntegrateExternalInterval (double s, double h, double tol, double* pre_alpha, bool minorstep)
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

lqc::Vector4d SimpsonErrorIntegrator::GetFromTransferFunction (double p_d)
{
  lqc::Vector4d ret;
  lqc::Vector3d p = minpos + p_d * normalized_step;
  if (!transfer_function || !volume) ret = lqc::Vector4d (0.0);
  else ret = transfer_function->Get (volume_evaluator->GetValueFromVolume (volume, lqc::Vector3f (p.x, p.y, p.z)));
  return ret;
}