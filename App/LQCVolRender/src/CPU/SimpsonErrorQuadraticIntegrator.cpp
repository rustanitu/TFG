#include "SimpsonErrorQuadraticIntegrator.h"

#include "VolumeEvaluator.h"

#include <iostream>
#include <cmath>
#include <cerrno>
#include <cfenv>
#include <cstring>

//http://martin.ankerl.com/2012/01/25/optimized-approximative-pow-in-c-and-cpp/

///Reutilizar não apenas amostras, mas também avaliações de intervalos
int SimpsonErrorQuadraticIntegrator::s_IntegrativeExternalTimes = 0;
int SimpsonErrorQuadraticIntegrator::s_IntegrativeInternalTimes = 0;

int SimpsonErrorQuadraticIntegrator::s_RecalculateIntegrativeExternalTimes = 0;
int SimpsonErrorQuadraticIntegrator::s_RecalculateIntegrativeInternalTimes = 0;

bool SimpsonErrorQuadraticIntegrator::ColorErrorEvalFunc (glm::dvec4 a, glm::dvec4 b, double tol)
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

SimpsonErrorQuadraticIntegrator::SimpsonErrorQuadraticIntegrator (VolumeEvaluator* veva)
{
  volume_evaluator = veva;
  m_integrate_method = 1;
}

SimpsonErrorQuadraticIntegrator::~SimpsonErrorQuadraticIntegrator ()
{}

void SimpsonErrorQuadraticIntegrator::Reset ()
{
  m_clc_proj_internal_error = 0;
  m_clc_proj_external_error = glm::dvec4 (0);
}

void SimpsonErrorQuadraticIntegrator::Init(glm::dvec3 minp, glm::dvec3 maxp, vr::Volume* vol, vr::TransferFunction* tf)
{
  volume = vol;
  transfer_function = tf;

  minpos = minp;
  maxpos = maxp;
  normalized_step = glm::normalize (maxpos - minpos);

  pre_integrated = 0.0;
  color = glm::dvec4 (0);
}

void SimpsonErrorQuadraticIntegrator::Integrate (double s0, double s1, double tol, double h0)
{
  //switch (m_integrate_method)
  //{
  //case 0:
  //  IntegrateError (s0, s1, tol, h0);
  //  break;
  //case 1:
    IntegrateProjection (s0, s1, tol, h0);
  //  break;
  //default:
  //  break;
  //}
}

void SimpsonErrorQuadraticIntegrator::IntegrateError (double s0, double s1, double tol, double h0)
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
    //CalculateInternalError (s, h, (tol_int * h) / (s1 - s0));

    //Calcular integral externa
    double H_error = (tol_ext * h) / (s1 - s0);
    if (!IntegrateExternalInterval (s, h, H_error, &pre_integrated))
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

void SimpsonErrorQuadraticIntegrator::IntegrateProjection (double s0, double s1, double tol, double h0)
{
  double tol_int = tol, tol_ext = tol;
  double h = std::min (h0, s1 - s0), hproj = 0;
  double s = s0;
  Cminpost = GetFromTransferFunction (s);
  while (s < s1)
  {
    h = std::max (h, hproj);
    h = std::min (h, s1 - s);
    h = std::min (h, 10.0);
    minpost = s;

    //Calcular integral interna
    double h_error = (tol_int * h) / (s1 - s0);
    hproj = std::min (std::pow (((15.0*h_error) / IntegrateInternalInterval (s, h, h_error)), 0.25) * h, 2.0 * h);
    hproj = std::min (hproj, s1 - s);
    h = std::min (h, hproj);
    //CalculateInternalError (s, h, (tol_int * h) / (s1 - s0));

    //Calcular integral externa
    double se1 = s + h;
    double H = h;
    while (s < se1)
    {
      double H_error = (tol_ext * H) / (s1 - s0);
      if (!IntegrateExternalInterval (s, H, H_error, &pre_integrated))
      {
        H = std::pow (((15.0*H_error) / MaxExternalError (external_error_aux)), 0.25) * H;

        double a, c, b;
        a = s; b = s + H; c = (a + b) / 2.0;
        glm::dvec4 gftd_c = GetFromTransferFunction (c);
        glm::dvec4 gftd_b = GetFromTransferFunction (b);

        glm::dvec4 fa = ExtenalEvaluation (a, Cminpost);
        glm::dvec4 fc = ExtenalEvaluation (c, gftd_c);
        //glm::dvec4 fd = ExtenalEvaluation ((a + c) / 2.0, GetFromTransferFunction ((a + c) / 2.0));
        //glm::dvec4 fe = ExtenalEvaluation ((c + b) / 2.0, GetFromTransferFunction ((c + b) / 2.0));

        glm::dvec4 fb = ExtenalEvaluationMiddle (b, gftd_b, gftd_c, &pre_integrated);

        glm::dvec4 S = SimpsonRule<glm::dvec4> (fa, fc, fb, H);
        color += S;
        //glm::dvec4 Sleft = SimpsonRule<glm::dvec4> (fa, fd, fc, h / 2.0);
        //glm::dvec4 Sright = SimpsonRule<glm::dvec4> (fc, fe, fb, h / 2.0);
        //color += (Sleft + Sright) + ((Sleft + Sright) - S) / 15.0;

        Cminpost = gftd_b;
        minpost = b;
      }
      s = s + H;
    }

  }
}

double SimpsonErrorQuadraticIntegrator::IntegrateInternalInterval (double s, double h, double tol)
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

void SimpsonErrorQuadraticIntegrator::CalculateInternalError (double s, double h, double tol)
{
  double d, c, e, b;
  b = s + h; c = (s + b) / 2.0;
  d = (s + c) / 2.0; e = (c + b) / 2.0;

  double fa = GetFromTransferFunction (s).w;
  double fd = GetFromTransferFunction (d).w;
  double fc = GetFromTransferFunction (c).w;
  double fe = GetFromTransferFunction (e).w;
  double fb = GetFromTransferFunction (b).w;

  double S = SimpsonRule<double> (fa, fc, fb, h);
  double Sleft = SimpsonRule<double> (fa, fd, fc, h / 2.0);
  double Sright = SimpsonRule<double> (fc, fe, fb, h / 2.0);

  if (fabs ((Sleft + Sright) - S) > tol * 15.0)
    m_clc_proj_internal_error += fabs ((Sleft + Sright - S)) - (tol * 15.0);
}

void SimpsonErrorQuadraticIntegrator::CalculateExternalError (double s, double h, double tol)
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

  if (!ColorErrorEvalFunc (S, S2, tol))
  {
    tol = 15.0 * tol;

    double error_r = fabs (S.x - S2.x);
    double error_g = fabs (S.y - S2.y);
    double error_b = fabs (S.z - S2.z);
    double error_a = fabs (S.w - S2.w);

    if (error_r > tol)
      m_clc_proj_external_error.x += error_r - tol;
    if (error_g > tol)
      m_clc_proj_external_error.y += error_g - tol;
    if (error_b > tol)
      m_clc_proj_external_error.z += error_b - tol;
    if (error_a > tol)
      m_clc_proj_external_error.w += error_a - tol;
  }
}

glm::dvec4 SimpsonErrorQuadraticIntegrator::ExtenalEvaluation (double p_d, glm::dvec4 C, double* pre_alpha)
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

glm::dvec4 SimpsonErrorQuadraticIntegrator::ExtenalEvaluationMiddle (double p_d, glm::dvec4 C, glm::dvec4 Cmid, double* pre_alpha)
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

bool SimpsonErrorQuadraticIntegrator::IntegrateExternalInterval (double s, double h, double tol, double* pre_alpha)
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

  if (ColorErrorEvalFunc (S, S2, tol))
  {
    color += S2 + (S2 - S) / 15.0;
    (*pre_alpha) = pre_a;
    Cminpost = gf_b;
    minpost = s + h;
    return true;
  }
  return false;
}

glm::dvec4 SimpsonErrorQuadraticIntegrator::GetFromTransferFunction (double p_d)
{
  glm::dvec4 ret;
  glm::dvec3 p = minpos + p_d * normalized_step;
  if (!transfer_function || !volume) ret = glm::dvec4 (0.0);
  else ret = transfer_function->Get(volume_evaluator->GetValueFromVolume(volume, lqc::Vector3f(p.x, p.y, p.z)));
  return ret;
}