#include "SimpsonHalfIterateIntegrator.h"

#include "VolumeEvaluator.h"
#include "SimpsonEvaluation.h"

void SimpsonHalfIterateIntegrator::CoupledIntegration (double s0, double s1, double tol, double h0, double hmin, double hmax)
{
  tol = 15.0 * tol;

  double tol_int = tol, tol_ext = tol,
    h = h0, hproj = h0;

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
#ifdef USE_HMAX
      h = std::min (std::min (hproj, hmax), s1 - anchor);
#else
      h = std::min (hproj, s1 - anchor);
#endif
      tol = tol_int_multiplier * h;

      tf_d = GetFromTransferFunction (anchor + h * 0.25);
      tf_c = GetFromTransferFunction (anchor + h * 0.50);
      tf_e = GetFromTransferFunction (anchor + h * 0.75);
      tf_b = GetFromTransferFunction (anchor + h);

      h_6 = h / 6.0;
      h_12 = h_6 * 0.5;

      Salfa = h_6 * (anchor_color.w + 4.0 * tf_c.w + tf_b.w);
      S2left = h_12 * (anchor_color.w + 4.0 * tf_d.w + tf_c.w);
      S2alfa = S2left + (h_12 * (tf_c.w + 4.0 * tf_e.w + tf_b.w));

      if (abs (S2alfa - Salfa) <= tol)
        hproj = 2.0 * h;
      else if (h <= hmin)
        hproj = hmin;
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
          tf_d = GetFromTransferFunction (anchor + h * 0.25);
          tf_e = GetFromTransferFunction (anchor + h * 0.75);

          Salfa = S2left;
          S2left = h_12 * (anchor_color.w + 4.0 * tf_d.w + tf_c.w);
          S2alfa = S2left + (h_12 * (tf_c.w + 4.0 * tf_e.w + tf_b.w));
        } while (h > hmin && abs (S2alfa - Salfa) > tol);
        hproj = std::max (hmin, h);
      }
    }
    //Integral Interna
    /////////////////////////////////////////////////
    
    /////////////////////////////////////////////////
    //Integral Externa
    {
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
        if (h <= hmin || (abs (S2S.w) <= tol && abs (S2S.x) <= tol && abs (S2S.y) <= tol && abs (S2S.z) <= tol))
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
            F_b = F_c;
            F_c = F_d;

            d = anchor + h * 0.25;
            e = anchor + h * 0.75;

            tf_d = GetFromTransferFunction (d);
            tf_e = GetFromTransferFunction (e);
            F_d = ExternalEvaluation (d, tf_d, GetFromTransferFunction (anchor + h * 0.125).w);
            F_e = ExternalEvaluation (e, tf_e, GetFromTransferFunction (anchor + h * 0.375).w);

            S = S2Eleft;
            S2Eleft = h_12 * (F_a + 4.0 * F_d + F_c);
            S2 = S2Eleft + h_12 * (F_c + 4.0 * F_e + F_b);

            S2S = S2 - S;
          } while (h > hmin && (abs (S2S.w) > tol || abs (S2S.x) > tol || abs (S2S.y) > tol || abs (S2S.z) > tol));

          color += S2 + S2S / 15.0;
          S2alfa = h_12 * (anchor_color.w + 4.0 * tf_d.w + 2.0 * tf_c.w + 4.0 * tf_e.w + tf_b.w);
          pre_integrated = pre_integrated + (S2alfa + (S2alfa - (h_6 * (anchor_color.w + 4.0 * tf_c.w + tf_b.w))) / 15.0);

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
          PrintExternalStepSize (h);
#endif

          anchor_color = tf_b;
          anchor = anchor + h;
          F_a = F_b;

          h = std::max (hmin, h);
          
          //Start the External loop integration
          do
          {
            h = std::min (h, se1 - anchor);
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

            h_6 = h / 6.0;
            h_12 = h_6 * 0.5;
            
            S = h_6 * (F_a + 4.0 * F_c + F_b);
            S2Eleft = h_12 * (F_a + 4.0 * F_d + F_c);
            S2 = S2Eleft + h_12 * (F_c + 4.0 * F_e + F_b);

            S2S = S2 - S;
            if (abs (S2S.w) <= tol && abs (S2S.x) <= tol && abs (S2S.y) <= tol && abs (S2S.z) <= tol)
            {
              color += S2 + S2S / 15.0;

              S2alfa = h_12 * (anchor_color.w + 4.0 * tf_d.w + 2.0 * tf_c.w + 4.0 * tf_e.w + tf_b.w);
              pre_integrated = pre_integrated + (S2alfa + (S2alfa - (h_6 * (anchor_color.w + 4.0 * tf_c.w + tf_b.w))) / 15.0);

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
              PrintExternalStepSize (h);
#endif

              anchor_color = tf_b;
              anchor = anchor + h;
              F_a = F_b;

              if (se1 <= anchor) break;

              h = h * 2.0;
            }
            else if (h <= hmin)
            {
              color += S2 + S2S / 15.0;
              S2alfa = h_12 * (anchor_color.w + 4.0 * tf_d.w + 2.0 * tf_c.w + 4.0 * tf_e.w + tf_b.w);
              pre_integrated = pre_integrated + (S2alfa + (S2alfa - (h_6 * (anchor_color.w + 4.0 * tf_c.w + tf_b.w))) / 15.0);

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
              PrintExternalStepSize (h);
#endif

              anchor_color = tf_b;
              anchor = anchor + h;
              F_a = F_b;

              if (se1 <= anchor) break;

              h = hmin;
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
                F_b = F_c;
                F_c = F_d;

                d = anchor + h * 0.25;
                e = anchor + h * 0.75;

                tf_d = GetFromTransferFunction (d);
                tf_e = GetFromTransferFunction (e);
                F_d = ExternalEvaluation (d, tf_d, GetFromTransferFunction (anchor + h * 0.125).w);
                F_e = ExternalEvaluation (e, tf_e, GetFromTransferFunction (anchor + h * 0.375).w);

                S = S2Eleft;
                S2Eleft = h_12 * (F_a + 4.0 * F_d + F_c);
                S2 = S2Eleft + h_12 * (F_c + 4.0 * F_e + F_b);

                S2S = S2 - S;
              } while (h > hmin && (abs (S2S.w) > tol || abs (S2S.x) > tol || abs (S2S.y) > tol || abs (S2S.z) > tol));

              color += S2 + S2S / 15.0;
              
              S2alfa = h_12 * (anchor_color.w + 4.0 * tf_d.w + 2.0 * tf_c.w + 4.0 * tf_e.w + tf_b.w);
              pre_integrated = pre_integrated + (S2alfa + (S2alfa - (h_6 * (anchor_color.w + 4.0 * tf_c.w + tf_b.w))) / 15.0);

#ifdef ANALYSIS__RGBA_ALONG_THE_RAY
              PrintExternalStepSize (h);
#endif

              anchor_color = tf_b;
              anchor = anchor + h;
              F_a = F_b;

              h = std::max (hmin, h);
            }
          } while (true);
        } 
      }
    }
    //Integral Externa
    /////////////////////////////////////////////////
  }
}
