//Internal Error Half, External Half

#include "SimpsonHalfQueueIntegrator.h"

#include "VolumeEvaluator.h"
#include "SimpsonEvaluation.h"

#include <iostream>
#include <fstream>
#include <cmath>
#include <cerrno>
#include <cfenv>
#include <cstring>

SimpsonHalfQueueIntegrator::SimpsonHalfQueueIntegrator (VolumeEvaluator* veva)
  : SimpsonIntegrator (veva) {}

SimpsonHalfQueueIntegrator::~SimpsonHalfQueueIntegrator ()
{
  Reset ();
}

void SimpsonHalfQueueIntegrator::Reset ()
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
}

void SimpsonHalfQueueIntegrator::PrintStepsEvaluation ()
{
#ifdef COMPUTE_STEPS_ALONG_EVALUATION
  printf ("Steps evaluation:\n");
  for (int i = 0; i < 11; i++)
    printf ("%d  h < %d\n", m_steps_evaluation[i], i + 1);
#endif
}

void SimpsonHalfQueueIntegrator::Init(glm::dvec3 minp, glm::dvec3 maxp, vr::Volume* vol, vr::TransferFunction* tf)
{
  SimpsonIntegrator::Init (minp, maxp, vol, tf);
}

void SimpsonHalfQueueIntegrator::Integrate (double s0, double s1, double tol, double h0, double hmint, double hmaxt)
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
    QueueExternalIntegral (s, h, (tol_ext * h) / htotal);
    s = s + h;
  }
}

void SimpsonHalfQueueIntegrator::IntegrateInternalProjected (double s0, double s1, double tol, double h0, double hmint, double hmaxt)
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

    h = InternalIntegralErrorByProjection (s, h, (tol_int * h) / htotal);
    QueueExternalIntegral (s, h, (tol_ext * h) / htotal);
    s = s + h;
  }
}

void SimpsonHalfQueueIntegrator::IntegrateIterated (double s0, double s1, double tol, double h0, double hmint, double hmaxt)
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
    if (f_int[0].w + f_int[1].w + f_int[2].w + f_int[3].w + f_int[4].w == 0.0)
    {
      anchor_color = f_int[4];
      anchor = s + h;
    }
    else
      IteratedExternalIntegral (h, (tol_ext * h) / htotal);

#ifdef COMPUTE_STEPS_ALONG_EVALUATION
    m_steps_evaluation[(int)h]++;
#endif

    s = s + h;
  }
}

double SimpsonHalfQueueIntegrator::InternalIntervalError (double s, double h)
{
  double      S = (h / 6.00) * (f_int[0].w + (4.0 * f_int[2].w) + f_int[4].w);
  double  Sleft = (h / 12.0) * (f_int[0].w + (4.0 * f_int[1].w) + f_int[2].w);
  double Sright = (h / 12.0) * (f_int[2].w + (4.0 * f_int[3].w) + f_int[4].w);

  return fabs ((Sleft + Sright) - S);
}

double SimpsonHalfQueueIntegrator::InternalIntegralErrorByProjection (double s, double h0, double error)
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
    hproj = std::pow (((15.0*h_error) / ierror), (1.0 / 3.0)) * h;
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

double SimpsonHalfQueueIntegrator::InternalIntegral (double s, double h0, double error)
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

bool SimpsonHalfQueueIntegrator::ColorErrorEvalFunc (glm::dvec4 a, glm::dvec4 b, double tol)
{
  tol = 15.0 * tol;
  
  return (tol >= fabs (a.x - b.x)
       && tol >= fabs (a.y - b.y)
       && tol >= fabs (a.z - b.z)
       && tol >= fabs (a.w - b.w));
}

void SimpsonHalfQueueIntegrator::QueueExternalIntegral (double s, double h0, double error)
{
  if (h0 == 0.0) return;
  glm::dvec4 F_a = ExternalEvaluationAnchor ();
  glm::dvec4 F_d, F_c, F_e, F_b;

  double a, d, c, e, b;
  glm::dvec4 S, Sleft, Sright, S2;
  double Salfa, S2alfa;

  a = s;
  b = s + h0;
  c = (a + b) /2.0;
  d = (a + c) / 2.0;
  e = (c + b) / 2.0;

  F_d = ExternalEvaluation (d, f_int[1], GetFromTransferFunction ((anchor + d) / 2.0));
  F_c = ExternalEvaluation (c, f_int[2], f_int[1]);
  F_e = ExternalEvaluation (e, f_int[3], GetFromTransferFunction ((anchor + e) / 2.0));
  F_b = ExternalEvaluation (b, f_int[4], f_int[2]);

  S = (h0 / 6.00) * (F_a + (4.0 * F_c) + F_b);
  Sleft = (h0 / 12.0) * (F_a + (4.0 * F_d) + F_c);
  Sright = (h0 / 12.0) * (F_c + (4.0 * F_e) + F_b);
  S2 = Sleft + Sright;

  if (ColorErrorEvalFunc (S, S2, error) || h0 <= hmin)
  {
    color += S2 + (S2 - S) / 15.0;

    Salfa = (h0 / 6.0) * (anchor_color.w + 4.0 * f_int[2].w + f_int[4].w);
    S2alfa =
      (h0 / 12.0) * (anchor_color.w + 4.0 * f_int[1].w + f_int[2].w) +
      (h0 / 12.0) * (f_int[2].w + 4.0 * f_int[3].w + f_int[4].w);

    pre_integrated = pre_integrated + S2alfa + (S2alfa - Salfa) / 15.0;

    anchor_color = f_int[4];
    anchor = s + h0;

    return;
  }

  //Pilha do simpson adaptativo
  simpsonrec queue[100];
  int q_n = 0;
  glm::dvec4 tf_d, tf_e;

  queue[q_n].h = h0 / 2.0;
  queue[q_n].tf_c = f_int[3];
  queue[q_n].tf_b = f_int[4];
  queue[q_n].F_c = ExternalEvaluation (s + (3.0 * h0 / 2.0), f_int[3], GetFromTransferFunction ((anchor + s + (3.0 * h0 / 2.0)) / 2.0));
  queue[q_n].F_b = ExternalEvaluation (s + h0, f_int[4], f_int[2]);
  q_n++;

  queue[q_n].h = h0 / 2.0;
  queue[q_n].tf_c = f_int[1];
  queue[q_n].tf_b = f_int[2];
  queue[q_n].F_c = ExternalEvaluation (s + h0 / 4.0, f_int[1], GetFromTransferFunction ((anchor + s + (h0 / 4.0)) / 2.0));
  queue[q_n].F_b = ExternalEvaluation (s + h0 / 2.0, f_int[2], f_int[1]);
  q_n++;

  while (q_n > 0)
  {
    simpsonrec r = queue[q_n - 1];

    c = anchor + (r.h / 2.0);
    d = anchor + (r.h / 4.0);
    e = anchor + (3.0 * r.h / 4.0);

    tf_d = GetFromTransferFunction (d);
    tf_e = GetFromTransferFunction (e);

    F_d = ExternalEvaluation (d, GetFromTransferFunction (d), GetFromTransferFunction ((anchor + d) / 2.0));
    F_e = ExternalEvaluation (e, GetFromTransferFunction (e), GetFromTransferFunction ((anchor + e) / 2.0));

         S = (r.h / 6.00) * (  F_a + (4.0 * r.F_c) + r.F_b);
     Sleft = (r.h / 12.0) * (  F_a + (4.0 *   F_d) + r.F_c);
    Sright = (r.h / 12.0) * (r.F_c + (4.0 *   F_e) + r.F_b);
        S2 = Sleft + Sright;

        if (ColorErrorEvalFunc (S2, S, (error * r.h / h0)) || r.h <= hmin)
    {
      color += S2 + (S2 - S) / 15.0;
      q_n--;

      Salfa = (r.h / 6.0) * (anchor_color.w + 4.0 * r.tf_c.w + r.tf_b.w);
      S2alfa =
        (r.h / 12.0) * (anchor_color.w + 4.0 * tf_d.w + r.tf_c.w) +
        (r.h / 12.0) * (r.tf_c.w + 4.0 * tf_e.w + r.tf_b.w);
      pre_integrated = pre_integrated + S2alfa + (S2alfa - Salfa) / 15.0;

      anchor_color = r.tf_b;
      anchor = anchor + r.h;
      F_a = ExternalEvaluationAnchor ();
    }
    else
    {
      queue[q_n - 1].h = r.h / 2.0;
      queue[q_n - 1].tf_c = tf_e;
      queue[q_n - 1].tf_b = r.tf_b;
      queue[q_n - 1].F_c = F_e;
      queue[q_n - 1].F_b = r.F_b;

      queue[q_n].h = r.h / 2.0;
      queue[q_n].tf_c = tf_d;
      queue[q_n].tf_b = r.tf_c;
      queue[q_n].F_c = F_d;
      queue[q_n].F_b = r.F_c;
      q_n++;
    }
  }
}

void SimpsonHalfQueueIntegrator::IteratedExternalIntegral (double h, double error)
{
  float tol_multiplier = error / h;
  float s1 = anchor + h;

  float S2alfa;
  glm::dvec4 S, S2;
  glm::dvec4 F_a = ExternalEvaluationAnchor ();
  glm::dvec4 F_d, F_c, F_e, F_b;
  float b, c, d, e;
  glm::dvec4 tf_d, tf_c, tf_e, tf_b;

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
    S2 = (h / 12.0) * (F_a + 4.0 * F_d + 2.0 * F_c + 4.0 * F_e + F_b);

    if (ColorErrorEvalFunc (S2, S, error) || h <= hmin)
    {
      color += S2 + (S2 - S) / 15.0;

      S2alfa = (h / 12.0) * (anchor_color.w + 4.0 * tf_d.w + 2.0 * tf_c.w + 4.0 * tf_e.w + tf_b.w);
      pre_integrated = pre_integrated + (S2alfa + (S2alfa - ((h / 6.0) * (anchor_color.w + 4.0 * tf_c.w + tf_b.w))) / 15.0);

      anchor_color = tf_b;
      anchor = anchor + h;
      F_a = F_b;

      h = std::min (h * 2.0, s1 - anchor);
      error = tol_multiplier * h;
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
        S2 = (h / 12.0) * (F_a + 4.0 * F_d + 2.0 * F_c + 4.0 * F_e + F_b);

      } while (!(ColorErrorEvalFunc (S2, S, error) || h <= hmin));

      color += S2 + (S2 - S) / 15.0;

      S2alfa = (h / 12.0) * (anchor_color.w + 4.0 * tf_d.w + 2.0 * tf_c.w + 4.0 * tf_e.w + tf_b.w);
      pre_integrated = pre_integrated + (S2alfa + (S2alfa - ((h / 6.0) * (anchor_color.w + 4.0 * tf_c.w + tf_b.w))) / 15.0);

      anchor_color = tf_b;
      anchor = anchor + h;
      F_a = F_b;
    }
  }
}