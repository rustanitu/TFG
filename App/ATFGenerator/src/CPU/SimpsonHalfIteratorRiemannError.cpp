//Internal Error Half, External Half

#include "SimpsonHalfIteratorRiemannError.h"

#include "VolumeEvaluator.h"
#include "SimpsonEvaluation.h"

#include <iostream>
#include <fstream>
#include <cmath>
#include <cerrno>
#include <cfenv>
#include <cstring>

SimpsonHalfIteratorRiemannError::SimpsonHalfIteratorRiemannError (VolumeEvaluator* veva)
  : SimpsonIntegrator (veva), rms (veva) {}

SimpsonHalfIteratorRiemannError::~SimpsonHalfIteratorRiemannError ()
{
  Reset ();
}

void SimpsonHalfIteratorRiemannError::Reset ()
{
#ifdef COMPUTE_STEPS_ALONG_EVALUATION
  m_steps_evaluation[0] = 0;
  m_steps_evaluation[1] = 0;
  m_steps_evaluation[2] = 0;
  m_steps_evaluation[3] = 0;
  m_steps_evaluation[4] = 0;
  m_steps_evaluation[5] = 0;
  m_steps_evaluation[6] = 0;
  m_steps_evaluation[7] = 0;
  m_steps_evaluation[8] = 0;
  m_steps_evaluation[9] = 0;
  m_steps_evaluation[10] = 0;
#endif

  m_clc_proj_newinternalprojection = 0;
  m_clc_proj_internal_error = 0;
  m_clc_proj_external_error = lqc::Vector4d (0);
}

void SimpsonHalfIteratorRiemannError::PrintStepsEvaluation ()
{
#ifdef COMPUTE_STEPS_ALONG_EVALUATION
  printf ("Steps evaluation:\n");
  for (int i = 0; i < 11; i++)
    printf ("%d  h < %d\n", m_steps_evaluation[i], i + 1);
#endif
}

void SimpsonHalfIteratorRiemannError::Init(lqc::Vector3d minp, lqc::Vector3d maxp, vr::Volume* vol, vr::TransferFunction* tf)
{
  SimpsonIntegrator::Init (minp, maxp, vol, tf);
  rms.Init (vol, tf, minpos, normalized_step);
}

void SimpsonHalfIteratorRiemannError::Integrate (double s0, double s1, double tol, double h0, double hmint, double hmaxt)
{
  hmax = hmaxt;
  hmin = hmint;
  double tol_int = tol, tol_ext = tol, h = h0, s = s0, htotal = s1 - s0;
  hproj = 0;
  anchor_color = GetFromTransferFunction (s);
  anchor = s;
  while (s1 > s)
  {
    h = std::max (std::max (h, hproj), hmin);
    h = std::min (std::min (h, hmax), s1 - s);

    h = InternalIntegral (s, h, (tol_int * h) / htotal);
    IteratedExternalIntegral (h, (tol_ext * h) / htotal);
 
#ifdef COMPUTE_STEPS_ALONG_EVALUATION
    m_steps_evaluation[(int)h]++;
#endif

    s = s + h;
  }
}

double SimpsonHalfIteratorRiemannError::InternalIntervalError (double s, double h)
{

  double      S = (h / 6.00) * (f_int[0].w + (4.0 * f_int[2].w) + f_int[4].w);
  double  Sleft = (h / 12.0) * (f_int[0].w + (4.0 * f_int[1].w) + f_int[2].w);
  double Sright = (h / 12.0) * (f_int[2].w + (4.0 * f_int[3].w) + f_int[4].w);

  return fabs ((Sleft + Sright) - S);
}

double SimpsonHalfIteratorRiemannError::InternalIntegral (double s, double h0, double error)
{
  double h = h0;
  double h_error = error;

  f_int[0] = anchor_color;
  f_int[1] = GetFromTransferFunction (s + (h / 4.0));
  f_int[2] = GetFromTransferFunction (s + (h / 2.0));
  f_int[3] = GetFromTransferFunction (s + (3.0*h / 4.0));
  f_int[4] = GetFromTransferFunction (s + h);

  double ierror = InternalIntervalError (s, h);

  if (ierror <= 15.0 * h_error || h <= hmin)
    hproj = 2.0 * h;
  else
  {
    while (ierror > 15.0*h_error)
    {
      h = h / 2.0;

      f_int[4] = f_int[2];
      f_int[2] = f_int[1];
      f_int[1] = GetFromTransferFunction (s + (h / 4.0));
      f_int[3] = GetFromTransferFunction (s + (3.0*h / 4.0));

      if (h <= hmin)
        break;
      
      h_error = (error * h) / h0;
      ierror = InternalIntervalError (s, h);
    }
    hproj = h;
  }
  return h;
}

bool SimpsonHalfIteratorRiemannError::ColorErrorEvalFunc (lqc::Vector4d a, lqc::Vector4d b, double tol)
{
  tol = 15.0 * tol;
  
  return (tol >= fabs (a.x - b.x)
       && tol >= fabs (a.y - b.y)
       && tol >= fabs (a.z - b.z)
       && tol >= fabs (a.w - b.w));
}

void SimpsonHalfIteratorRiemannError::IteratedExternalIntegral (double h, double error)
{
  float s1 = anchor + h;

  float S2alfa;
  lqc::Vector4d S, S2;
  lqc::Vector4d F_a = ExternalEvaluationAnchor ();
  lqc::Vector4d F_d, F_c, F_e, F_b;
  float b, c, d, e;
  lqc::Vector4d tf_d, tf_c, tf_e, tf_b;

  while (s1 > anchor)
  {
    b = anchor + h; c = (anchor + b) * 0.5;
    d = (anchor + c) * 0.5; e = (c + b) * 0.5;

    tf_d = GetFromTransferFunction (d);
    tf_c = GetFromTransferFunction (c);
    tf_e = GetFromTransferFunction (e);
    tf_b = GetFromTransferFunction (b);

    F_d = ExternalEvaluation (d, tf_d, GetFromTransferFunction ((anchor + d) * 0.5).w);
    F_c = ExternalEvaluation (c, tf_c, tf_d.w);
    F_e = ExternalEvaluation (e, tf_e, GetFromTransferFunction ((anchor + e) * 0.5).w);
    F_b = ExternalEvaluation (b, tf_b, tf_c.w);

    S = (h / 6.00) * (F_a + 4.0 * F_c + F_b);
    S2 = rms.EstipulateSum (anchor, anchor + h, 0.01);

    if (ColorErrorEvalFunc (S2, S, error) || h <= hmin)
    {
      color += S2 + (S2 - S) / 15.0;
      rms.Composite ();


      S2alfa = (h / 12.0) * (anchor_color.w + 4.0 * tf_d.w + 2.0 * tf_c.w + 4.0 * tf_e.w + tf_b.w);
      pre_integrated = pre_integrated + (S2alfa + (S2alfa - ((h / 6.0) * (anchor_color.w + 4.0 * tf_c.w + tf_b.w))) / 15.0);

      anchor_color = tf_b;
      anchor = anchor + h;
      F_a = F_b;

      h = std::min (h * 2.0, s1 - anchor);
    }
    else
    {
      do
      {
        h = h * 0.5;
        error = error * 0.5;

        tf_b = tf_c;
        tf_c = tf_d;

        d = anchor + h * 0.25;
        e = anchor + h * 0.75;

        tf_d = GetFromTransferFunction (d);
        tf_e = GetFromTransferFunction (e);

        F_b = F_c;
        F_c = F_d;

        F_d = ExternalEvaluation (d, tf_d, GetFromTransferFunction ((anchor + d) * 0.5).w);
        F_e = ExternalEvaluation (e, tf_e, GetFromTransferFunction ((anchor + e) * 0.5).w);

        S = (h / 6.00) * (F_a + 4.0 * F_c + F_b);
        S2 = rms.EstipulateSum (anchor, anchor + h, 0.01);

      } while (!(ColorErrorEvalFunc (S2, S, error) || h <= hmin));

      color += S2 + (S2 - S) / 15.0;
      rms.Composite ();

      S2alfa = (h / 12.0) * (anchor_color.w + 4.0 * tf_d.w + 2.0 * tf_c.w + 4.0 * tf_e.w + tf_b.w);
      pre_integrated = pre_integrated + (S2alfa + (S2alfa - ((h / 6.0) * (anchor_color.w + 4.0 * tf_c.w + tf_b.w))) / 15.0);

      anchor_color = tf_b;
      anchor = anchor + h;
      F_a = F_b;
    }
  }
}