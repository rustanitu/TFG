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

bool SimpsonErrorIntegrator::ColorErrorEvalFunc (glm::dvec4 a, glm::dvec4 b, double tol)
{
  tol = 15.0 * tol;

  double error_r = fabs (a.x - b.x);
  double error_g = fabs (a.y - b.y);
  double error_b = fabs (a.z - b.z);
  double error_a = fabs (a.w - b.w);

  external_error_aux = glm::dvec4 (error_r, error_g, error_b, error_a);

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
  m_clc_proj_external_error = glm::dvec4 (0);
}

void SimpsonErrorIntegrator::Init(glm::dvec3 minp, glm::dvec3 maxp, vr::Volume* vol, vr::TransferFunction* tf)
{
  volume = vol;
  transfer_function = tf;

  minpos = minp;
  maxpos = maxp;
  normalized_step = glm::normalize (maxpos - minpos);

  pre_integrated = 0.0;
  color = glm::dvec4 (0);
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
      glm::dvec4 gftd_c = GetFromTransferFunction (c);
      glm::dvec4 gftd_b = GetFromTransferFunction (b);

      glm::dvec4 fa = ExtenalEvaluation (a, Cminpost);
      glm::dvec4 fc = ExtenalEvaluation (c, gftd_c);
      //glm::dvec4 fd = ExtenalEvaluation ((a + c) / 2.0, GetFromTransferFunction ((a + c) / 2.0));
      //glm::dvec4 fe = ExtenalEvaluation ((c + b) / 2.0, GetFromTransferFunction ((c + b) / 2.0));
      glm::dvec4 fb = ExtenalEvaluationMiddle (b, gftd_b, gftd_c, &pre_integrated);

      glm::dvec4 S = SimpsonRule<glm::dvec4> (fa, fc, fb, h);
      color += S;
      //glm::dvec4 Sleft = SimpsonRule<glm::dvec4> (fa, fd, fc, h / 2.0);
      //glm::dvec4 Sright = SimpsonRule<glm::dvec4> (fc, fe, fb, h / 2.0);
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
  printf ("Proportional error: %lf\n", ret / (tol*15.0));
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

glm::dvec4 SimpsonErrorIntegrator::CalculateExternalError (double s, double h, double tol)
{
  glm::dvec4 err (0);
  double a, d, c, e, b;
  a = s; b = s + h; c = (a + b) / 2.0;
  d = (a + c) / 2.0; e = (c + b) / 2.0;

  double pre_a;
  glm::dvec4 gf_b = GetFromTransferFunction (b);
  glm::dvec4 gf_d = GetFromTransferFunction (d);
  glm::dvec4 gf_c = GetFromTransferFunction (c);

  glm::dvec4 fa = ExtenalEvaluation (a, Cminpost);
  glm::dvec4 fd = ExtenalEvaluation (d, gf_d);
  glm::dvec4 fc = ExtenalEvaluationMiddle (c, gf_c, gf_d);
  glm::dvec4 fe = ExtenalEvaluation (e, GetFromTransferFunction (e));
  glm::dvec4 fb = ExtenalEvaluationMiddle (b, gf_b, gf_c, &pre_a);

  glm::dvec4 S = SimpsonRule<glm::dvec4> (fa, fc, fb, h);
  glm::dvec4 Sleft = SimpsonRule<glm::dvec4> (fa, fd, fc, h / 2.0);
  glm::dvec4 Sright = SimpsonRule<glm::dvec4> (fc, fe, fb, h / 2.0);
  glm::dvec4 S2 = Sleft + Sright;

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

glm::dvec4 SimpsonErrorIntegrator::ExtenalEvaluation (double p_d, glm::dvec4 C, double* pre_alpha)
{
  double alpha = 0.0;
  double h = p_d - minpost;
  if (h != 0)
    alpha = (h / 6.0) * (Cminpost.w + 4.0 * GetFromTransferFunction ((minpost + p_d) / 2.0).w + C.w);
  
  double innerint = exp (-(pre_integrated + alpha));

  if (pre_alpha)
    (*pre_alpha) = pre_integrated + alpha;

  double alphachannel = C.w*innerint;
  return glm::dvec4 (
    alphachannel * C.x,
    alphachannel * C.y,
    alphachannel * C.z,
    alphachannel
    );
}

glm::dvec4 SimpsonErrorIntegrator::ExtenalEvaluationMiddle (double p_d, glm::dvec4 C, glm::dvec4 Cmid, double* pre_alpha)
{
  double alpha = 0.0;
  double h = p_d - minpost;
  if (h != 0)
    alpha = (h / 6.0) * (Cminpost.w + 4.0 * Cmid.w + C.w);

  double innerint = exp (-(pre_integrated + alpha));

  if (pre_alpha)
    (*pre_alpha) = pre_integrated + alpha;

  double alphachannel = C.w*innerint;
  return glm::dvec4 (
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
  glm::dvec4 gf_b = GetFromTransferFunction (b);
  glm::dvec4 gf_d = GetFromTransferFunction (d);
  glm::dvec4 gf_c = GetFromTransferFunction (c);

  glm::dvec4 fa = ExtenalEvaluation (a, Cminpost);
  glm::dvec4 fd = ExtenalEvaluation (d, gf_d);
  glm::dvec4 fc = ExtenalEvaluationMiddle (c, gf_c, gf_d);
  glm::dvec4 fe = ExtenalEvaluation (e, GetFromTransferFunction (e));
  glm::dvec4 fb = ExtenalEvaluationMiddle (b, gf_b, gf_c, &pre_a);

  glm::dvec4 S = SimpsonRule<glm::dvec4> (fa, fc, fb, h);
  glm::dvec4 Sleft = SimpsonRule<glm::dvec4> (fa, fd, fc, h / 2.0);
  glm::dvec4 Sright = SimpsonRule<glm::dvec4> (fc, fe, fb, h / 2.0);
  glm::dvec4 S2 = Sleft + Sright;

  if (ColorErrorEvalFunc (S, S2, tol) || minorstep)
  {
    color += S2 + (S2 - S) / 15.0;
    (*pre_alpha) = pre_a;
    Cminpost = gf_b;
    return true;
  }
  return false;
}

glm::dvec4 SimpsonErrorIntegrator::GetFromTransferFunction (double p_d)
{
  glm::dvec4 ret;
  glm::dvec3 p = minpos + p_d * normalized_step;
  if (!transfer_function || !volume) ret = glm::dvec4 (0.0);
  else ret = transfer_function->Get (volume_evaluator->GetValueFromVolume (volume, lqc::Vector3f (p.x, p.y, p.z)));
  return ret;
}