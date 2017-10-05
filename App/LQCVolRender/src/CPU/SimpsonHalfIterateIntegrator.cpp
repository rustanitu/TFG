//Internal Error Half, External Half

#include "SimpsonHalfIterateIntegrator.h"

#include "VolumeEvaluator.h"
#include "SimpsonEvaluation.h"

#include <iostream>
#include <fstream>
#include <cmath>
#include <cerrno>
#include <cfenv>
#include <cstring>

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
void SimpsonHalfIterateIntegrator::PrintExternalStepSize (double h)
{
  if (file_ext)
  {
    (*file_ext) 
      << std::setprecision (30) << anchor     << '\t'
      << std::setprecision (30) << anchor + h << '\t'
      << std::setprecision (30) << h          << '\t'
      << std::setprecision (30) << color.x    << '\t'
      << std::setprecision (30) << color.y    << '\t'
      << std::setprecision (30) << color.z    << '\t'
      << std::setprecision (30) << color.w    << '\n';
  }
}

void SimpsonHalfIterateIntegrator::PrintInternalStepSize (double h)
{
  if (file_int)
  {
    (*file_int)
      << std::setprecision (30) << anchor         << '\t'
      << std::setprecision (30) << anchor + h     << '\t'
      << std::setprecision (30) << h              << '\t';
  }
}

void SimpsonHalfIterateIntegrator::PrintInternalColor ()
{
  if (file_int)
  {
    (*file_int) 
      << std::setprecision (30) << pre_integrated << '\n';
  }
}
#endif

SimpsonHalfIterateIntegrator::SimpsonHalfIterateIntegrator (VolumeEvaluator* veva)
  : SimpsonIntegrator (veva) {}

SimpsonHalfIterateIntegrator::~SimpsonHalfIterateIntegrator ()
{
  Reset ();
}

void SimpsonHalfIterateIntegrator::Reset ()
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

void SimpsonHalfIterateIntegrator::PrintStepsEvaluation ()
{
#ifdef COMPUTE_STEPS_ALONG_EVALUATION
  printf ("Steps evaluation:\n");
  for (int i = 0; i < 11; i++)
    printf ("%d  h < %d\n", m_steps_evaluation[i], i + 1);
#endif
}

void SimpsonHalfIterateIntegrator::Init (glm::dvec3 minp, glm::dvec3 maxp, vr::Volume* vol, vr::TransferFunction* tf)
{
  SimpsonIntegrator::Init (minp, maxp, vol, tf);

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
  file_ext = file_int = NULL;
#endif
}

void SimpsonHalfIterateIntegrator::IntegrateSimple (double s0, double s1, double tol, double h0, double hmin, double hmax)
{
  tol = 15.0 * tol;

  double tol_int = tol, tol_ext = tol, h = h0;

  anchor = s0;
  anchor_color = GetFromTransferFunction (anchor);

  float Salfa, S2alfa;
  glm::dvec4 S, S2;
  float b, c, d, e;
  glm::dvec4 tf_d, tf_c, tf_e, tf_b, S2S;
  glm::dvec4 tf_ad, tf_ae;
  glm::dvec4 F_d, F_c, F_e, F_b;
  glm::dvec4 F_a = glm::dvec4 (anchor_color.w * anchor_color.x, anchor_color.w * anchor_color.y, anchor_color.w * anchor_color.z, anchor_color.w);

  double hproj = h;

  double h_6, h_12, S2left;
  glm::dvec4 S2Eleft;

  double tol_multiplier = tol_int / s1;

  while (s1 > anchor)
  {
    h = std::max (hproj, hmin);
    h = std::min (std::min (h, hmax), s1 - anchor);


    tol = tol_multiplier * h;
    //Integral Interna
    tf_d = GetFromTransferFunction (anchor + h * 0.25);
    tf_c = GetFromTransferFunction (anchor + h * 0.50);
    tf_e = GetFromTransferFunction (anchor + h * 0.75);
    tf_b = GetFromTransferFunction (anchor + h);

    h_6 = h / 6.00;
    h_12 = h_6 * 0.5;

    Salfa = h_6 * (anchor_color.w + 4.0 * tf_c.w + tf_b.w);
    S2left = h_12 * (anchor_color.w + 4.0 * tf_d.w + tf_c.w);
    S2alfa = S2left + (h_12 * (tf_c.w + 4.0 * tf_e.w + tf_b.w));

    if (abs (S2alfa - Salfa) <= tol || h <= hmin)
      hproj = 2.0 * h;
    else
    {
      do
      {
        h = h * 0.5;
        tol = tol * 0.5;
        h_12 = h_12 * 0.5;

        tf_b = tf_c;
        tf_c = tf_d;
        tf_d = GetFromTransferFunction (anchor + h * 0.25);
        tf_e = GetFromTransferFunction (anchor + h * 0.75);

        Salfa = S2left;
        S2left = h_12 * (anchor_color.w + 4.0 * tf_d.w + tf_c.w);
        S2alfa = S2left + (h_12 * (tf_c.w + 4.0 * tf_e.w + tf_b.w));
      } while (abs (S2alfa - Salfa) > tol && h > hmin);
      hproj = h;
      h_6 = h_12 * 2.0;
    }

#ifdef COMPUTE_STEPS_ALONG_EVALUATION
    m_steps_evaluation[(int)h]++;
#endif
#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
    PrintInternalStepSize (h);
#endif

    double se1 = anchor + h;

    h_6 = h / 6.00;
    h_12 = h_6 * 0.5;
    tol = tol_multiplier * h;

    while (true)
    {
      b = anchor + h; c = anchor + h * 0.5;
      d = anchor + h * 0.25; e = anchor + h * 0.75;

      tf_d = GetFromTransferFunction (d);
      tf_c = GetFromTransferFunction (c);
      tf_e = GetFromTransferFunction (e);
      tf_b = GetFromTransferFunction (b);

      F_d = ExternalEvaluation (d, tf_d, GetFromTransferFunction (anchor + h * 0.125).w);
      F_c = ExternalEvaluation (c, tf_c, tf_d.w);
      F_e = ExternalEvaluation (e, tf_e, GetFromTransferFunction (anchor + h * 0.375).w);
      F_b = ExternalEvaluation (b, tf_b, tf_c.w);

      S = h_6 * (F_a + 4.0 * F_c + F_b);
      S2Eleft = h_12 * (F_a + 4.0 * F_d + F_c);
      S2 = S2Eleft + h_12 * (F_c + 4.0 * F_e + F_b);

      S2S = S2 - S;
      if ((tol >= abs (S2S.w) && tol >= abs (S2S.x) && tol >= abs (S2S.y) && tol >= abs (S2S.z)) || h <= hmin)
      {
        color += S2 + S2S / 15.0;

        S2alfa = h_12 * (anchor_color.w + 4.0 * tf_d.w + 2.0 * tf_c.w + 4.0 * tf_e.w + tf_b.w);
        pre_integrated = pre_integrated + (S2alfa + (S2alfa - h_6 * (anchor_color.w + 4.0 * tf_c.w + tf_b.w)) / 15.0);

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
        PrintExternalStepSize (h);
#endif

        anchor_color = tf_b;
        anchor = anchor + h;
        F_a = F_b;

        if (se1 == anchor)
          break;

        h = std::min (h * 2.0, se1 - anchor);
        
        tol = tol_multiplier * h;
        h_6 = h / 6.00;
        h_12 = h_6 * 0.5;
      }
      else
      {
        do
        {
          h = h * 0.5;
          tol = tol * 0.5;
          h_6 = h_12;
          h_12 = h_12 * 0.5;

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

          S = S2Eleft;
          S2Eleft = h_12 * (F_a + 4.0 * F_d + F_c);
          S2 = S2Eleft + h_12 * (F_c + 4.0 * F_e + F_b);

          S2S = S2 - S;
        } while (!((tol >= abs (S2S.w) && tol >= abs (S2S.x) && tol >= abs (S2S.y) && tol >= abs (S2S.z)) || h <= hmin));

        color += S2 + S2S / 15.0;

        S2alfa = h_12 * (anchor_color.w + 4.0 * tf_d.w + 2.0 * tf_c.w + 4.0 * tf_e.w + tf_b.w);
        pre_integrated = pre_integrated + (S2alfa + (S2alfa - h_6 * (anchor_color.w + 4.0 * tf_c.w + tf_b.w)) / 15.0);

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
        PrintExternalStepSize (h);
#endif

        anchor_color = tf_b;
        anchor = anchor + h;
        F_a = F_b;
      }
    }
#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
    PrintInternalColor ();
#endif
  }
}

void SimpsonHalfIterateIntegrator::IntegrateSimpleExtStep (double s0, double s1, double tol, double h0, double hmin, double hmax)
{
  tol = 15.0 * tol;

  double tol_int = tol, tol_ext = tol, h = h0;

  anchor = s0;
  anchor_color = GetFromTransferFunction (anchor);

  float Salfa, S2alfa;
  glm::dvec4 S, S2;
  float b, c, d, e;
  glm::dvec4 tf_d, tf_c, tf_e, tf_b, S2S;
  glm::dvec4 tf_ad, tf_ae;
  glm::dvec4 F_d, F_c, F_e, F_b;
  glm::dvec4 F_a = glm::dvec4 (anchor_color.w * anchor_color.x, anchor_color.w * anchor_color.y, anchor_color.w * anchor_color.z, anchor_color.w);

  double hproj = h;
  double hext = h;

  double h_6, h_12, S2left;
  glm::dvec4 S2Eleft;

  double tol_multiplier = tol_int / s1;

  while (s1 > anchor)
  {
    h = std::max (hproj, hmin);
    h = std::min (std::min (h, hmax), s1 - anchor);


    tol = tol_multiplier * h;
    //Integral Interna
    tf_d = GetFromTransferFunction (anchor + h * 0.25);
    tf_c = GetFromTransferFunction (anchor + h * 0.50);
    tf_e = GetFromTransferFunction (anchor + h * 0.75);
    tf_b = GetFromTransferFunction (anchor + h);

    h_6 = h / 6.00;
    h_12 = h_6 * 0.5;

    Salfa = h_6 * (anchor_color.w + 4.0 * tf_c.w + tf_b.w);
    S2left = h_12 * (anchor_color.w + 4.0 * tf_d.w + tf_c.w);
    S2alfa = S2left + (h_12 * (tf_c.w + 4.0 * tf_e.w + tf_b.w));

    if (abs (S2alfa - Salfa) <= tol || h <= hmin)
      hproj = 2.0 * h;
    else
    {
      do
      {
        h = h * 0.5;
        tol = tol * 0.5;
        h_12 = h_12 * 0.5;

        tf_b = tf_c;
        tf_c = tf_d;
        tf_d = GetFromTransferFunction (anchor + h * 0.25);
        tf_e = GetFromTransferFunction (anchor + h * 0.75);

        Salfa = S2left;
        S2left = h_12 * (anchor_color.w + 4.0 * tf_d.w + tf_c.w);
        S2alfa = S2left + (h_12 * (tf_c.w + 4.0 * tf_e.w + tf_b.w));
      } while (abs (S2alfa - Salfa) > tol && h > hmin);
      hproj = h;
      h_6 = h_12 * 2.0;
    }

#ifdef COMPUTE_STEPS_ALONG_EVALUATION
    m_steps_evaluation[(int)h]++;
#endif
#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
    PrintInternalStepSize (h);
#endif

    double se1 = anchor + h;
    if (h <= hext)
    {
      hext = h;
    }
    else h = hext;

    h_6 = h / 6.00;
    h_12 = h_6 * 0.5;
    tol = tol_multiplier * h;

    while (true)
    {
      b = anchor + h; c = anchor + h * 0.5;
      d = anchor + h * 0.25; e = anchor + h * 0.75;

      tf_d = GetFromTransferFunction (d);
      tf_c = GetFromTransferFunction (c);
      tf_e = GetFromTransferFunction (e);
      tf_b = GetFromTransferFunction (b);

      F_d = ExternalEvaluation (d, tf_d, GetFromTransferFunction (anchor + h * 0.125).w);
      F_c = ExternalEvaluation (c, tf_c, tf_d.w);
      F_e = ExternalEvaluation (e, tf_e, GetFromTransferFunction (anchor + h * 0.375).w);
      F_b = ExternalEvaluation (b, tf_b, tf_c.w);

      S = h_6 * (F_a + 4.0 * F_c + F_b);
      S2Eleft = h_12 * (F_a + 4.0 * F_d + F_c);
      S2 = S2Eleft + h_12 * (F_c + 4.0 * F_e + F_b);

      S2S = S2 - S;
      if ((tol >= abs (S2S.w) && tol >= abs (S2S.x) && tol >= abs (S2S.y) && tol >= abs (S2S.z)) || h <= hmin)
      {
        color += S2 + S2S / 15.0;

        S2alfa = h_12 * (anchor_color.w + 4.0 * tf_d.w + 2.0 * tf_c.w + 4.0 * tf_e.w + tf_b.w);
        pre_integrated = pre_integrated + (S2alfa + (S2alfa - h_6 * (anchor_color.w + 4.0 * tf_c.w + tf_b.w)) / 15.0);

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
        PrintExternalStepSize (h);
#endif

        anchor_color = tf_b;
        anchor = anchor + h;
        F_a = F_b;

        if (se1 == anchor)
          break;

        hext = 2.0 * h;
        h = std::min (h * 2.0, se1 - anchor);

        tol = tol_multiplier * h;
        h_6 = h / 6.00;
        h_12 = h_6 * 0.5;
      }
      else
      {
        do
        {
          h = h * 0.5;
          tol = tol * 0.5;
          h_6 = h_12;
          h_12 = h_12 * 0.5;

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

          S = S2Eleft;
          S2Eleft = h_12 * (F_a + 4.0 * F_d + F_c);
          S2 = S2Eleft + h_12 * (F_c + 4.0 * F_e + F_b);

          S2S = S2 - S;
        } while (!((tol >= abs (S2S.w) && tol >= abs (S2S.x) && tol >= abs (S2S.y) && tol >= abs (S2S.z)) || h <= hmin));

        color += S2 + S2S / 15.0;

        S2alfa = h_12 * (anchor_color.w + 4.0 * tf_d.w + 2.0 * tf_c.w + 4.0 * tf_e.w + tf_b.w);
        pre_integrated = pre_integrated + (S2alfa + (S2alfa - h_6 * (anchor_color.w + 4.0 * tf_c.w + tf_b.w)) / 15.0);

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
        PrintExternalStepSize (h);
#endif

        anchor_color = tf_b;
        anchor = anchor + h;
        F_a = F_b;
        hext = h;
      }
    }

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
    PrintInternalColor ();
#endif
  }
}

void SimpsonHalfIterateIntegrator::IntegrateComplexExtStep (double s0, double s1, double tol, double h0, double hmin, double hmax)
{
  tol = 15.0 * tol;

  double tol_int = tol, tol_ext = tol, h = h0;

  anchor = s0;
  anchor_color = GetFromTransferFunction (anchor);

  float Salfa, S2alfa;
  glm::dvec4 S, S2;
  float b, c, d, e;
  glm::dvec4 tf_d, tf_c, tf_e, tf_b, S2S;
  glm::dvec4 tf_ad, tf_ae;
  glm::dvec4 F_d, F_c, F_e, F_b;
  glm::dvec4 F_a = glm::dvec4 (anchor_color.w * anchor_color.x, anchor_color.w * anchor_color.y, anchor_color.w * anchor_color.z, anchor_color.w);

  double hproj = h;
  double hext = h;

  double h_6, h_12, S2left;
  glm::dvec4 S2Eleft;

  double tol_multiplier = tol_int / s1;

  while (s1 > anchor)
  {
    h = std::max (hproj, hmin);
    h = std::min (std::min (h, hmax), s1 - anchor);


    tol = tol_multiplier * h;
    //Integral Interna
    tf_d = GetFromTransferFunction (anchor + h * 0.25);
    tf_c = GetFromTransferFunction (anchor + h * 0.50);
    tf_e = GetFromTransferFunction (anchor + h * 0.75);
    tf_b = GetFromTransferFunction (anchor + h);

    h_6 = h / 6.00;
    h_12 = h_6 * 0.5;

    Salfa = h_6 * (anchor_color.w + 4.0 * tf_c.w + tf_b.w);
    S2left = h_12 * (anchor_color.w + 4.0 * tf_d.w + tf_c.w);
    S2alfa = S2left + (h_12 * (tf_c.w + 4.0 * tf_e.w + tf_b.w));

    if (abs (S2alfa - Salfa) <= tol || h <= hmin)
      hproj = 2.0 * h;
    else
    {
      do
      {
        h = h * 0.5;
        tol = tol * 0.5;
        h_12 = h_12 * 0.5;

        tf_b = tf_c;
        tf_c = tf_d;
        tf_d = GetFromTransferFunction (anchor + h * 0.25);
        tf_e = GetFromTransferFunction (anchor + h * 0.75);

        Salfa = S2left;
        S2left = h_12 * (anchor_color.w + 4.0 * tf_d.w + tf_c.w);
        S2alfa = S2left + (h_12 * (tf_c.w + 4.0 * tf_e.w + tf_b.w));
      } while (abs (S2alfa - Salfa) > tol && h > hmin);
      hproj = h;
      h_6 = h_12 * 2.0;
    }

#ifdef COMPUTE_STEPS_ALONG_EVALUATION
    m_steps_evaluation[(int)h]++;
#endif
#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
    PrintInternalStepSize (h);
#endif

    double se1 = anchor + h;
    //O passo que deve ser integrado deve ser igual ao passo
    // feito na integral interna
    if (h <= hext)
    {
      hext = hproj;

      if (S2alfa == 0.0)
      {

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
        PrintExternalStepSize (h);
#endif
        anchor_color = tf_b;
        anchor = anchor + h;
        F_a = glm::dvec4 (0.0);
      }
      else
      {
        b = anchor + h; c = anchor + h * 0.5;
        d = anchor + h * 0.25; e = anchor + h * 0.75;

        F_d = ExternalEvaluation (d, tf_d, GetFromTransferFunction (anchor + h * 0.125).w);
        F_c = ExternalEvaluation (c, tf_c, tf_d.w);
        F_e = ExternalEvaluation (e, tf_e, GetFromTransferFunction (anchor + h * 0.375).w);
        F_b = ExternalEvaluation (b, tf_b, tf_c.w);

        S = h_6 * (F_a + 4.0 * F_c + F_b);
        S2Eleft = h_12 * (F_a + 4.0 * F_d + F_c);
        S2 = S2Eleft + h_12 * (F_c + 4.0 * F_e + F_b);

        S2S = S2 - S;
        if ((tol >= abs (S2S.w) && tol >= abs (S2S.x) && tol >= abs (S2S.y) && tol >= abs (S2S.z)) || h <= hmin)
        {
          color += S2 + S2S / 15.0;

          pre_integrated = pre_integrated + (S2alfa + (S2alfa - Salfa) / 15.0);

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
          PrintExternalStepSize (h);
#endif
          anchor_color = tf_b;
          anchor = anchor + h;
          F_a = F_b;
        }
        else
        {
          double se1 = anchor + h;
          do
          {
            h = h * 0.5;
            tol = tol * 0.5;
            h_6 = h_12;
            h_12 = h_12 * 0.5;

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

            S = S2Eleft;
            S2Eleft = h_12 * (F_a + 4.0 * F_d + F_c);
            S2 = S2Eleft + h_12 * (F_c + 4.0 * F_e + F_b);

            S2S = S2 - S;
          } while (!((tol >= abs (S2S.w) && tol >= abs (S2S.x) && tol >= abs (S2S.y) && tol >= abs (S2S.z)) || h <= hmin));

          color += S2 + S2S / 15.0;

          S2alfa = h_12 * (anchor_color.w + 4.0 * tf_d.w + 2.0 * tf_c.w + 4.0 * tf_e.w + tf_b.w);
          pre_integrated = pre_integrated + (S2alfa + (S2alfa - h_6 * (anchor_color.w + 4.0 * tf_c.w + tf_b.w)) / 15.0);

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
          PrintExternalStepSize (h);
#endif

          anchor_color = tf_b;
          anchor = anchor + h;
          F_a = F_b;
          hext = h;
          
          while (true)
          {
            b = anchor + h; c = anchor + h * 0.5;
            d = anchor + h * 0.25; e = anchor + h * 0.75;

            tf_d = GetFromTransferFunction (d);
            tf_c = GetFromTransferFunction (c);
            tf_e = GetFromTransferFunction (e);
            tf_b = GetFromTransferFunction (b);

            F_d = ExternalEvaluation (d, tf_d, GetFromTransferFunction (anchor + h * 0.125).w);
            F_c = ExternalEvaluation (c, tf_c, tf_d.w);
            F_e = ExternalEvaluation (e, tf_e, GetFromTransferFunction (anchor + h * 0.375).w);
            F_b = ExternalEvaluation (b, tf_b, tf_c.w);

            h_6 = h / 6.00;
            h_12 = h_6 * 0.5;

            S = h_6 * (F_a + 4.0 * F_c + F_b);
            S2Eleft = h_12 * (F_a + 4.0 * F_d + F_c);
            S2 = S2Eleft + h_12 * (F_c + 4.0 * F_e + F_b);

            S2S = S2 - S;
            if ((tol >= abs (S2S.w) && tol >= abs (S2S.x) && tol >= abs (S2S.y) && tol >= abs (S2S.z)) || h <= hmin)
            {
              color += S2 + S2S / 15.0;

              S2alfa = h_12 * (anchor_color.w + 4.0 * tf_d.w + 2.0 * tf_c.w + 4.0 * tf_e.w + tf_b.w);
              pre_integrated = pre_integrated + (S2alfa + (S2alfa - h_6 * (anchor_color.w + 4.0 * tf_c.w + tf_b.w)) / 15.0);

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
              PrintExternalStepSize (h);
#endif

              anchor_color = tf_b;
              anchor = anchor + h;
              F_a = F_b;

              if (se1 <= anchor)
                break;

              h = std::min (h * 2.0, se1 - anchor);
              tol = tol_multiplier * h;
            }
            else
            {
              do
              {
                h = h * 0.5;
                tol = tol * 0.5;
                h_6 = h_12;
                h_12 = h_12 * 0.5;

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

                S = S2Eleft;
                S2Eleft = h_12 * (F_a + 4.0 * F_d + F_c);
                S2 = S2Eleft + h_12 * (F_c + 4.0 * F_e + F_b);

                S2S = S2 - S;
              } while (!((tol >= abs (S2S.w) && tol >= abs (S2S.x) && tol >= abs (S2S.y) && tol >= abs (S2S.z)) || h <= hmin));

              color += S2 + S2S / 15.0;

              S2alfa = h_12 * (anchor_color.w + 4.0 * tf_d.w + 2.0 * tf_c.w + 4.0 * tf_e.w + tf_b.w);
              pre_integrated = pre_integrated + (S2alfa + (S2alfa - h_6 * (anchor_color.w + 4.0 * tf_c.w + tf_b.w)) / 15.0);

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
              PrintExternalStepSize (h);
#endif

              anchor_color = tf_b;
              anchor = anchor + h;
              F_a = F_b;
              hext = h;
            }

          }
        }
      }
    }
    //O passo que deve ser integrado deve ser menor ao passo
    // feito na integral interna
    else
    {
      h = hext;

      h_6 = h / 6.00;
      h_12 = h_6 * 0.5;
      tol = tol_multiplier * h;

      while (true)
      {
        b = anchor + h; c = anchor + h * 0.5;
        d = anchor + h * 0.25; e = anchor + h * 0.75;

        tf_d = GetFromTransferFunction (d);
        tf_c = GetFromTransferFunction (c);
        tf_e = GetFromTransferFunction (e);
        tf_b = GetFromTransferFunction (b);

        F_d = ExternalEvaluation (d, tf_d, GetFromTransferFunction (anchor + h * 0.125).w);
        F_c = ExternalEvaluation (c, tf_c, tf_d.w);
        F_e = ExternalEvaluation (e, tf_e, GetFromTransferFunction (anchor + h * 0.375).w);
        F_b = ExternalEvaluation (b, tf_b, tf_c.w);

        S = h_6 * (F_a + 4.0 * F_c + F_b);
        S2Eleft = h_12 * (F_a + 4.0 * F_d + F_c);
        S2 = S2Eleft + h_12 * (F_c + 4.0 * F_e + F_b);

        S2S = S2 - S;
        if ((tol >= abs (S2S.w) && tol >= abs (S2S.x) && tol >= abs (S2S.y) && tol >= abs (S2S.z)) || h <= hmin)
        {
          color += S2 + S2S / 15.0;

          S2alfa = h_12 * (anchor_color.w + 4.0 * tf_d.w + 2.0 * tf_c.w + 4.0 * tf_e.w + tf_b.w);
          pre_integrated = pre_integrated + (S2alfa + (S2alfa - h_6 * (anchor_color.w + 4.0 * tf_c.w + tf_b.w)) / 15.0);

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
          PrintExternalStepSize (h);
#endif

          anchor_color = tf_b;
          anchor = anchor + h;
          F_a = F_b;

          if (se1 == anchor)
            break;

          hext = 2.0 * h;
          h = std::min (h * 2.0, se1 - anchor);

          tol = tol_multiplier * h;
          h_6 = h / 6.00;
          h_12 = h_6 * 0.5;
        }
        else
        {
          do
          {
            h = h * 0.5;
            tol = tol * 0.5;
            h_6 = h_12;
            h_12 = h_12 * 0.5;

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

            S = S2Eleft;
            S2Eleft = h_12 * (F_a + 4.0 * F_d + F_c);
            S2 = S2Eleft + h_12 * (F_c + 4.0 * F_e + F_b);

            S2S = S2 - S;
          } while (!((tol >= abs (S2S.w) && tol >= abs (S2S.x) && tol >= abs (S2S.y) && tol >= abs (S2S.z)) || h <= hmin));

          color += S2 + S2S / 15.0;

          S2alfa = h_12 * (anchor_color.w + 4.0 * tf_d.w + 2.0 * tf_c.w + 4.0 * tf_e.w + tf_b.w);
          pre_integrated = pre_integrated + (S2alfa + (S2alfa - h_6 * (anchor_color.w + 4.0 * tf_c.w + tf_b.w)) / 15.0);

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
          PrintExternalStepSize (h);
#endif

          anchor_color = tf_b;
          anchor = anchor + h;
          F_a = F_b;
          hext = h;
        }
      }
    }
#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
    PrintInternalColor ();
#endif
  }
}

void SimpsonHalfIterateIntegrator::IntegrateExp (double s0, double s1, double tol, double h0, double hmin, double hmax)
{
  tol = 15.0 * tol;
  pre_integrated = 1.0;

  double tol_int = tol, tol_ext = tol, h = h0;

  anchor = s0;
  anchor_color = GetFromTransferFunction (anchor);

  float Salfa, S2alfa;
  glm::dvec4 S, S2;
  float b, c, d, e;
  glm::dvec4 tf_d, tf_c, tf_e, tf_b, S2S;
  glm::dvec4 F_d, F_c, F_e, F_b;
  glm::dvec4 F_a = glm::dvec4 (anchor_color.w * anchor_color.x, anchor_color.w * anchor_color.y, anchor_color.w * anchor_color.z, anchor_color.w);

  double hproj = h;

  double h_6, h_12, S2left;
  glm::dvec4 S2Eleft;

  double tol_multiplier = tol_int / s1;

  while (s1 > anchor)
  {
    h = std::max (hproj, hmin);
    h = std::min (std::min (h, hmax), s1 - anchor);


    tol = tol_multiplier * h;
    //Integral Interna
    tf_d = GetFromTransferFunction (anchor + h * 0.25);
    tf_c = GetFromTransferFunction (anchor + h * 0.50);
    tf_e = GetFromTransferFunction (anchor + h * 0.75);
    tf_b = GetFromTransferFunction (anchor + h);

    h_6 = h / 6.00;
    h_12 = h_6 * 0.5;

    Salfa = h_6 * (anchor_color.w + 4.0 * tf_c.w + tf_b.w);
    S2left = h_12 * (anchor_color.w + 4.0 * tf_d.w + tf_c.w);
    S2alfa = S2left + (h_12 * (tf_c.w + 4.0 * tf_e.w + tf_b.w));

    if (abs (S2alfa - Salfa) <= tol || h <= hmin)
      hproj = 2.0 * h;
    else
    {
      do
      {
        h = h * 0.5;
        tol = tol * 0.5;
        h_12 = h_12 * 0.5;

        tf_b = tf_c;
        tf_c = tf_d;
        tf_d = GetFromTransferFunction (anchor + h * 0.25);
        tf_e = GetFromTransferFunction (anchor + h * 0.75);

        Salfa = S2left;
        S2left = h_12 * (anchor_color.w + 4.0 * tf_d.w + tf_c.w);
        S2alfa = S2left + (h_12 * (tf_c.w + 4.0 * tf_e.w + tf_b.w));
      } while (abs (S2alfa - Salfa) > tol && h > hmin);
      hproj = h;
      h_6 = h_12 * 2.0;
    }

#ifdef COMPUTE_STEPS_ALONG_EVALUATION
    m_steps_evaluation[(int)h]++;
#endif

    if (anchor_color.w + tf_d.w + tf_c.w + tf_e.w + tf_b.w == 0.0)
    {
      anchor_color = tf_b;
      anchor = anchor + h;
      F_a = glm::dvec4 (0.0);
    }
    else
    {
      b = anchor + h; c = anchor + h * 0.5;
      d = anchor + h * 0.25; e = anchor + h * 0.75;

      F_d = ExternalEvaluationApprox (d, tf_d, GetFromTransferFunction (anchor + h * 0.125));
      F_c = ExternalEvaluationApprox (c, tf_c, tf_d);
      F_e = ExternalEvaluationApprox (e, tf_e, GetFromTransferFunction (anchor + h * 0.375));
      F_b = ExternalEvaluationApprox (b, tf_b, tf_c);

      S = h_6 * (F_a + 4.0 * F_c + F_b);
      S2Eleft = h_12 * (F_a + 4.0 * F_d + F_c);
      S2 = S2Eleft + h_12 * (F_c + 4.0 * F_e + F_b);

      S2S = S2 - S;
      if ((tol >= abs (S2S.w) && tol >= abs (S2S.x) && tol >= abs (S2S.y) && tol >= abs (S2S.z)) || h <= hmin)
      {
        color += S2 + S2S / 15.0;

        double x = -h_12 * (anchor_color.w + 4.0 * tf_d.w + 2.0 * tf_c.w + 4.0 * tf_e.w + tf_b.w);
        pre_integrated = pre_integrated * (1.0 + x + (x*x) / 2.0 + (x*x*x) / 6.0);

        anchor_color = tf_b;
        anchor = anchor + h;
        F_a = F_b;
      }
      else
      {
        double se1 = anchor + h;

        h = h * 0.5;
        tol = tol * 0.5;

        while (se1 > anchor)
        {
          b = anchor + h; c = anchor + h * 0.5;
          d = anchor + h * 0.25; e = anchor + h * 0.75;

          tf_d = GetFromTransferFunction (d);
          tf_c = GetFromTransferFunction (c);
          tf_e = GetFromTransferFunction (e);
          tf_b = GetFromTransferFunction (b);

          F_d = ExternalEvaluationApprox (d, tf_d, GetFromTransferFunction (anchor + h * 0.125));
          F_c = ExternalEvaluationApprox (c, tf_c, tf_d);
          F_e = ExternalEvaluationApprox (e, tf_e, GetFromTransferFunction (anchor + h * 0.375));
          F_b = ExternalEvaluationApprox (b, tf_b, tf_c);

          h_6 = h / 6.00;
          h_12 = h_6 * 0.5;

          S = h_6 * (F_a + 4.0 * F_c + F_b);
          S2Eleft = h_12 * (F_a + 4.0 * F_d + F_c);
          S2 = S2Eleft + h_12 * (F_c + 4.0 * F_e + F_b);

          S2S = S2 - S;
          if ((tol >= abs (S2S.w) && tol >= abs (S2S.x) && tol >= abs (S2S.y) && tol >= abs (S2S.z)) || h <= hmin)
          {
            color += S2 + S2S / 15.0;

            double x = -h_12 * (anchor_color.w + 4.0 * tf_d.w + 2.0 * tf_c.w + 4.0 * tf_e.w + tf_b.w);
            pre_integrated = pre_integrated * (1.0 + x + (x*x) / 2.0 + (x*x*x) / 6.0);

            anchor_color = tf_b;
            anchor = anchor + h;
            F_a = F_b;

            h = std::min (h * 2.0, se1 - anchor);
            tol = tol_multiplier * h;
          }
          else
          {
            do
            {
              h = h * 0.5;
              tol = tol * 0.5;
              h_6 = h_12;
              h_12 = h_12 * 0.5;

              tf_b = tf_c;
              tf_c = tf_d;

              d = anchor + h * 0.25;
              e = anchor + h * 0.75;

              tf_d = GetFromTransferFunction (d);
              tf_e = GetFromTransferFunction (e);

              F_b = F_c;
              F_c = F_d;

              F_d = ExternalEvaluationApprox (d, tf_d, GetFromTransferFunction ((anchor + d) * 0.5));
              F_e = ExternalEvaluationApprox (e, tf_e, GetFromTransferFunction ((anchor + e) * 0.5));

              S = S2Eleft;
              S2Eleft = h_12 * (F_a + 4.0 * F_d + F_c);
              S2 = S2Eleft + h_12 * (F_c + 4.0 * F_e + F_b);

              S2S = S2 - S;
            } while (!((tol >= abs (S2S.w) && tol >= abs (S2S.x) && tol >= abs (S2S.y) && tol >= abs (S2S.z)) || h <= hmin));

            color += S2 + S2S / 15.0;

            double x = -h_12 * (anchor_color.w + 4.0 * tf_d.w + 2.0 * tf_c.w + 4.0 * tf_e.w + tf_b.w);
            pre_integrated = pre_integrated * (1.0 + x + (x*x) / 2.0 + (x*x*x) / 6.0);

            anchor_color = tf_b;
            anchor = anchor + h;
            F_a = F_b;
          }

        }
      }
    }
  }
}

void SimpsonHalfIterateIntegrator::IntegrateSeparated (double s0, double s1, double tol, double h0, double hmin, double hmax)
{
  tol = 15.0 * tol;

  double  tol_int = tol, tol_ext = tol,
    hext = h0, hint = h0, hproj = h0;

  anchor = s0;
  anchor_color = GetFromTransferFunction (anchor);
  glm::dvec4 F_a = ExternalEvaluationAnchor ();

  double tol_int_multiplier = tol_int / s1;
  double tol_ext_multiplier = tol_ext / s1;

  double b, c, d, e, h_6, h_12,
    Salfa, S2alfa, S2left;
  glm::dvec4 tf_d, tf_c, tf_e, tf_b,
    F_d, F_c, F_e, F_b,
    S, S2, S2S, S2Eleft;

  while (s1 > anchor)
  {
    /////////////////////////////////////////////////
    //Integral Interna
    {
      hint = std::max (hproj, hmin);
      hint = std::min (std::min (hint, hmax), s1 - anchor);

      tol = tol_int_multiplier * hint;

      tf_d = GetFromTransferFunction (anchor + hint * 0.25);
      tf_c = GetFromTransferFunction (anchor + hint * 0.50);
      tf_e = GetFromTransferFunction (anchor + hint * 0.75);
      tf_b = GetFromTransferFunction (anchor + hint);

      h_6 = hint / 6.0;
      h_12 = h_6 * 0.5;

      Salfa = h_6 * (anchor_color.w + 4.0 * tf_c.w + tf_b.w);
      S2left = h_12 * (anchor_color.w + 4.0 * tf_d.w + tf_c.w);
      S2alfa = S2left + (h_12 * (tf_c.w + 4.0 * tf_e.w + tf_b.w));

      if (abs (S2alfa - Salfa) <= tol || hint <= hmin)
        hproj = 2.0 * hint;
      else
      {
        do
        {
          hint = hint * 0.5;

          if (hint <= hmin)
          {
            hint = hmin;
            tol = tol_int_multiplier * hint;

            tf_d = GetFromTransferFunction (anchor + hint * 0.25);
            tf_c = GetFromTransferFunction (anchor + hint * 0.50);
            tf_e = GetFromTransferFunction (anchor + hint * 0.75);
            tf_b = GetFromTransferFunction (anchor + hint);

            h_12 = hint / 12.0;

            Salfa = (h_12 * 2.0) * (anchor_color.w + 4.0 * tf_c.w + tf_b.w);
            S2left = h_12 * (anchor_color.w + 4.0 * tf_d.w + tf_c.w);
            S2alfa = S2left + (h_12 * (tf_c.w + 4.0 * tf_e.w + tf_b.w));
            break;
          }

          tol = tol * 0.5;
          h_12 = h_12 * 0.5;

          tf_b = tf_c;
          tf_c = tf_d;
          tf_d = GetFromTransferFunction (anchor + hint * 0.25);
          tf_e = GetFromTransferFunction (anchor + hint * 0.75);

          Salfa = S2left;
          S2left = h_12 * (anchor_color.w + 4.0 * tf_d.w + tf_c.w);
          S2alfa = S2left + (h_12 * (tf_c.w + 4.0 * tf_e.w + tf_b.w));
        } while (abs (S2alfa - Salfa) > tol);
        hproj = hint;
      }
    }

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
    PrintInternalStepSize (hint);
#endif

    /////////////////////////////////////////////////
    //Integral Externa
    {
      double h = hext;
      //Caso o intervalo da integral externa for maior 
      // que o intervalo da integral interna
      if (hint <= h)
      {
        h = hint;

        if (S2alfa == 0.0)
        {
#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
          PrintExternalStepSize (h);
#endif
          anchor_color = tf_b;
          anchor = anchor + h;
          F_a = glm::dvec4 (0.0);
          hext = std::max (hext, hproj);
          continue;
        }
        else
        {
          h_6 = h_12 * 2.0;

          b = anchor + h; c = anchor + h * 0.5;
          d = anchor + h * 0.25; e = anchor + h * 0.75;

          F_d = ExternalEvaluation (d, tf_d, GetFromTransferFunction (anchor + h * 0.125).w);
          F_c = ExternalEvaluation (c, tf_c, tf_d.w);
          F_e = ExternalEvaluation (e, tf_e, GetFromTransferFunction (anchor + h * 0.375).w);
          F_b = ExternalEvaluation (b, tf_b, tf_c.w);

          S = h_6 * (F_a + 4.0 * F_c + F_b);
          S2Eleft = h_12 * (F_a + 4.0 * F_d + F_c);
          S2 = S2Eleft + h_12 * (F_c + 4.0 * F_e + F_b);

          S2S = S2 - S;
          if ((tol >= abs (S2S.w) && tol >= abs (S2S.x) &&
            tol >= abs (S2S.y) && tol >= abs (S2S.z)) || h <= hmin)
          {
            color += S2 + S2S / 15.0;

            pre_integrated = pre_integrated + (S2alfa + (S2alfa - Salfa) / 15.0);

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
            PrintExternalStepSize (h);
#endif

            anchor_color = tf_b;
            anchor = anchor + h;
            F_a = F_b;

            hext = std::max (hext, hproj);
            continue;
          }
        }
        h = h * 0.5;
      }

      //O passo que deve ser integrado deve ser menor ao passo
      // feito na integral interna
      tol = tol_ext_multiplier * h;
      h_6 = h / 6.0;
      h_12 = h_6 * 0.5;

      double s1 = anchor + hint;
      while (true)
      {
        b = anchor + h; c = anchor + h * 0.5;
        d = anchor + h * 0.25; e = anchor + h * 0.75;

        tf_d = GetFromTransferFunction (d);
        tf_c = GetFromTransferFunction (c);
        tf_e = GetFromTransferFunction (e);
        tf_b = GetFromTransferFunction (b);

        F_d = ExternalEvaluation (d, tf_d, GetFromTransferFunction (anchor + h * 0.125).w);
        F_c = ExternalEvaluation (c, tf_c, tf_d.w);
        F_e = ExternalEvaluation (e, tf_e, GetFromTransferFunction (anchor + h * 0.375).w);
        F_b = ExternalEvaluation (b, tf_b, tf_c.w);

        S = h_6 * (F_a + 4.0 * F_c + F_b);
        S2Eleft = h_12 * (F_a + 4.0 * F_d + F_c);
        S2 = S2Eleft + h_12 * (F_c + 4.0 * F_e + F_b);

        S2S = S2 - S;
        if ((tol >= abs (S2S.w) && tol >= abs (S2S.x) && tol >= abs (S2S.y) && tol >= abs (S2S.z)) || h <= hmin)
        {
          color += S2 + S2S / 15.0;

          double S2alfa = h_12 * (anchor_color.w + 4.0 * tf_d.w + 2.0 * tf_c.w + 4.0 * tf_e.w + tf_b.w);
          pre_integrated = pre_integrated + (S2alfa + (S2alfa - h_6 * (anchor_color.w + 4.0 * tf_c.w + tf_b.w)) / 15.0);

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
          PrintExternalStepSize (h);
#endif

          anchor_color = tf_b;
          anchor = anchor + h;
          F_a = F_b;

          if (s1 <= anchor) break;

          double h_2 = 2.0 * h;
          double s1_anchor = s1 - anchor;

          hext = h_2;
          h = std::min (h_2, s1_anchor);


          h_6 = h / 6.0;
          h_12 = h_6 * 0.5;
          tol = tol_ext_multiplier * h;
        }
        else
        {
          do
          {
            h = h * 0.5;

            if (h <= hmin)
            {
              h = hmin;
              tol = tol_ext_multiplier * h;

              b = anchor + h; c = anchor + h * 0.5;
              d = anchor + h * 0.25; e = anchor + h * 0.75;

              tf_d = GetFromTransferFunction (d);
              tf_c = GetFromTransferFunction (c);
              tf_e = GetFromTransferFunction (e);
              tf_b = GetFromTransferFunction (b);

              F_d = ExternalEvaluation (d, tf_d, GetFromTransferFunction (anchor + h * 0.125).w);
              F_c = ExternalEvaluation (c, tf_c, tf_d.w);
              F_e = ExternalEvaluation (e, tf_e, GetFromTransferFunction (anchor + h * 0.375).w);
              F_b = ExternalEvaluation (b, tf_b, tf_c.w);

              h_12 = h / 12.0;

              S = (h_12 * 2.0) * (F_a + 4.0 * F_c + F_b);
              S2Eleft = h_12 * (F_a + 4.0 * F_d + F_c);
              S2 = S2Eleft + h_12 * (F_c + 4.0 * F_e + F_b);

              S2S = S2 - S;
              break;
            }

            tol = tol * 0.5;
            h_12 = h_12 * 0.5;

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

            S = S2Eleft;
            S2Eleft = h_12 * (F_a + 4.0 * F_d + F_c);
            S2 = S2Eleft + h_12 * (F_c + 4.0 * F_e + F_b);

            S2S = S2 - S;
          } while (!((tol >= abs (S2S.w) && tol >= abs (S2S.x) && tol >= abs (S2S.y) && tol >= abs (S2S.z))));

          color += S2 + S2S / 15.0;

          h_6 = h_12 * 2.0;

          double S2alfa = h_12 * (anchor_color.w + 4.0 * tf_d.w + 2.0 * tf_c.w + 4.0 * tf_e.w + tf_b.w);
          pre_integrated = pre_integrated + (S2alfa + (S2alfa - h_6 * (anchor_color.w + 4.0 * tf_c.w + tf_b.w)) / 15.0);

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
          PrintExternalStepSize (h);
#endif

          anchor_color = tf_b;
          anchor = anchor + h;
          F_a = F_b;
          hext = h;
        }
      }
    }
#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
    PrintInternalColor ();
#endif
  }
}

void SimpsonHalfIterateIntegrator::IntegrateScount (double s0, double s1, double tol, double h0, double hmin, double hmax)
{
  tol = 15.0 * tol;

  double tol_int = tol, tol_ext = tol, h = h0;

  anchor = s0;
  anchor_color = GetFromTransferFunction (anchor);

  float Salfa, S2alfa;
  glm::dvec4 S, S2;
  float b, c, d, e;
  glm::dvec4 tf_d, tf_c, tf_e, tf_b, S2S;
  glm::dvec4 tf_ad, tf_ae;
  glm::dvec4 F_d, F_c, F_e, F_b;
  glm::dvec4 F_a = glm::dvec4 (anchor_color.w * anchor_color.x, anchor_color.w * anchor_color.y, anchor_color.w * anchor_color.z, anchor_color.w);

  double  hproj = h;

  double h_6, h_12, S2left;
  glm::dvec4 S2Eleft;

  double tol_multiplier = tol_int / s1;

  while (s1 > anchor)
  {
    h = std::max (hproj, hmin);
    h = std::min (std::min (h, hmax), s1 - anchor);


    tol = tol_multiplier * h;
    //Integral Interna
    tf_d = GetFromTransferFunction (anchor + h * 0.25);
    tf_c = GetFromTransferFunction (anchor + h * 0.50);
    tf_e = GetFromTransferFunction (anchor + h * 0.75);
    tf_b = GetFromTransferFunction (anchor + h);

    h_6 = h / 6.00;
    h_12 = h_6 * 0.5;

    Salfa = h_6 * (anchor_color.w + 4.0 * tf_c.w + tf_b.w);
    S2left = h_12 * (anchor_color.w + 4.0 * tf_d.w + tf_c.w);
    S2alfa = S2left + (h_12 * (tf_c.w + 4.0 * tf_e.w + tf_b.w));

    if (abs (S2alfa - Salfa) <= tol || h <= hmin)
      hproj = 2.0 * h;
    else
    {
      do
      {
        h = h * 0.5;
        
        if (h <= hmin)
        {
          h = hmin;
          tol = tol_multiplier * h;

          tf_d = GetFromTransferFunction (anchor + h * 0.25);
          tf_c = GetFromTransferFunction (anchor + h * 0.50);
          tf_e = GetFromTransferFunction (anchor + h * 0.75);
          tf_b = GetFromTransferFunction (anchor + h);

          h_12 = h / 12.0;
          h_6 = h_12 * 2.0;

          Salfa = h_6 * (anchor_color.w + 4.0 * tf_c.w + tf_b.w);
          S2left = h_12 * (anchor_color.w + 4.0 * tf_d.w + tf_c.w);
          S2alfa = S2left + (h_12 * (tf_c.w + 4.0 * tf_e.w + tf_b.w));
          break;
        }

        tol = tol * 0.5;
        h_12 = h_12 * 0.5;

        tf_b = tf_c;
        tf_c = tf_d;
        tf_d = GetFromTransferFunction (anchor + h * 0.25);
        tf_e = GetFromTransferFunction (anchor + h * 0.75);


        Salfa = h_6 * (anchor_color.w + 4.0 * tf_c.w + tf_b.w);
        S2left = h_12 * (anchor_color.w + 4.0 * tf_d.w + tf_c.w);
        S2alfa = S2left + (h_12 * (tf_c.w + 4.0 * tf_e.w + tf_b.w));
      } while (abs (S2alfa - Salfa) > tol);
      hproj = h;
      h_6 = h_12 * 2.0;
    }

#ifdef COMPUTE_STEPS_ALONG_EVALUATION
    m_steps_evaluation[(int)h]++;
#endif
#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
    PrintInternalStepSize (h);
#endif

    if (anchor_color.w + tf_d.w + tf_c.w + tf_e.w + tf_b.w == 0.0)
    {

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
      PrintExternalStepSize (h);
#endif
      anchor_color = tf_b;
      anchor = anchor + h;
      F_a = glm::dvec4 (0.0);
    }
    else
    {
      b = anchor + h; c = anchor + h * 0.5;
      d = anchor + h * 0.25; e = anchor + h * 0.75;

      F_d = ExternalEvaluation (d, tf_d, GetFromTransferFunction (anchor + h * 0.125).w);
      F_c = ExternalEvaluation (c, tf_c, tf_d.w);
      F_e = ExternalEvaluation (e, tf_e, GetFromTransferFunction (anchor + h * 0.375).w);
      F_b = ExternalEvaluation (b, tf_b, tf_c.w);

      S = h_6 * (F_a + 4.0 * F_c + F_b);
      S2Eleft = h_12 * (F_a + 4.0 * F_d + F_c);
      S2 = S2Eleft + h_12 * (F_c + 4.0 * F_e + F_b);

      S2S = S2 - S;
      if ((tol >= abs (S2S.w) && tol >= abs (S2S.x) && tol >= abs (S2S.y) && tol >= abs (S2S.z)) || h <= hmin)
      {
        color += S2 + S2S / 15.0;

        pre_integrated = pre_integrated + (S2alfa + (S2alfa - Salfa) / 15.0);

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
        PrintExternalStepSize (h);
#endif
        anchor_color = tf_b;
        anchor = anchor + h;
        F_a = F_b;
      }
      else
      {
        double se1 = anchor + h;

        h = h * 0.5;
        tol = tol * 0.5;

        while (true)
        {
          b = anchor + h; c = anchor + h * 0.5;
          d = anchor + h * 0.25; e = anchor + h * 0.75;

          tf_d = GetFromTransferFunction (d);
          tf_c = GetFromTransferFunction (c);
          tf_e = GetFromTransferFunction (e);
          tf_b = GetFromTransferFunction (b);

          F_d = ExternalEvaluation (d, tf_d, GetFromTransferFunction (anchor + h * 0.125).w);
          F_c = ExternalEvaluation (c, tf_c, tf_d.w);
          F_e = ExternalEvaluation (e, tf_e, GetFromTransferFunction (anchor + h * 0.375).w);
          F_b = ExternalEvaluation (b, tf_b, tf_c.w);

          h_6 = h / 6.00;
          h_12 = h_6 * 0.5;

          S = h_6 * (F_a + 4.0 * F_c + F_b);
          S2Eleft = h_12 * (F_a + 4.0 * F_d + F_c);
          S2 = S2Eleft + h_12 * (F_c + 4.0 * F_e + F_b);

          S2S = S2 - S;



          if ((tol >= abs (S2S.w) && tol >= abs (S2S.x) && tol >= abs (S2S.y) && tol >= abs (S2S.z)) || h <= hmin)
          {
            color += S2 + S2S / 15.0;

            S2alfa = h_12 * (anchor_color.w + 4.0 * tf_d.w + 2.0 * tf_c.w + 4.0 * tf_e.w + tf_b.w);
            pre_integrated = pre_integrated + (S2alfa + (S2alfa - h_6 * (anchor_color.w + 4.0 * tf_c.w + tf_b.w)) / 15.0);

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
            PrintExternalStepSize (h);
#endif

            anchor_color = tf_b;
            anchor = anchor + h;
            F_a = F_b;

            if (se1 <= anchor)
              break;

            h = std::min (h * 2.0, se1 - anchor);
            tol = tol_multiplier * h;
          }
          else
          {
            do
            {
              h = h * 0.5;

              if (h <= hmin)
              {
                h = hmin;
                tol = tol_multiplier * h;

                b = anchor + h; c = anchor + h * 0.5;
                d = anchor + h * 0.25; e = anchor + h * 0.75;

                tf_d = GetFromTransferFunction (d);
                tf_c = GetFromTransferFunction (c);
                tf_e = GetFromTransferFunction (e);
                tf_b = GetFromTransferFunction (b);

                F_d = ExternalEvaluation (d, tf_d, GetFromTransferFunction (anchor + h * 0.125).w);
                F_c = ExternalEvaluation (c, tf_c, tf_d.w);
                F_e = ExternalEvaluation (e, tf_e, GetFromTransferFunction (anchor + h * 0.375).w);
                F_b = ExternalEvaluation (b, tf_b, tf_c.w);

                h_12 = h / 12.0;
                h_6 = h_12 * 2.0;

                S = h_6 * (F_a + 4.0 * F_c + F_b);
                S2Eleft = h_12 * (F_a + 4.0 * F_d + F_c);
                S2 = S2Eleft + h_12 * (F_c + 4.0 * F_e + F_b);

                S2S = S2 - S;
                break;
              }


              tol = tol * 0.5;
              h_6 = h_12;
              h_12 = h_12 * 0.5;

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

              S = S2Eleft;
              S2Eleft = h_12 * (F_a + 4.0 * F_d + F_c);
              S2 = S2Eleft + h_12 * (F_c + 4.0 * F_e + F_b);

              S2S = S2 - S;
            } while (!((tol >= abs (S2S.w) && tol >= abs (S2S.x) && tol >= abs (S2S.y) && tol >= abs (S2S.z))));

            color += S2 + S2S / 15.0;

            S2alfa = h_12 * (anchor_color.w + 4.0 * tf_d.w + 2.0 * tf_c.w + 4.0 * tf_e.w + tf_b.w);
            pre_integrated = pre_integrated + (S2alfa + (S2alfa - h_6 * (anchor_color.w + 4.0 * tf_c.w + tf_b.w)) / 15.0);

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
            PrintExternalStepSize (h);
#endif

            anchor_color = tf_b;
            anchor = anchor + h;
            F_a = F_b;
          }

        }
      }
    }

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
    PrintInternalColor ();
#endif
  }
}