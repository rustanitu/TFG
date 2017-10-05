#include "SimpsonHalfIterateIntegrator.h"

#include "VolumeEvaluator.h"
#include "SimpsonEvaluation.h"

void SimpsonHalfIterateIntegrator::DecoupledIntegration (double s0, double s1, double tol, double h0, double hmin, double hmax)
{
  tol = 15.0 * tol;

  double tol_int = tol, tol_ext = tol,
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
#ifdef USE_HMAX
      hint = std::min (std::min (hproj, hmax), s1 - anchor);
#else
      hint = std::min (hproj, s1 - anchor);
#endif
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

      if (abs (S2alfa - Salfa) <= tol)
        hproj = 2.0 * hint;
      else if (hint <= hmin)
        hproj = hmin;
      else
      {
        do
        {
          hint = hint * 0.5;
          tol = tol * 0.5;
          h_6 = h_12;
          h_12 = h_12 * 0.5;

          tf_b = tf_c;
          tf_c = tf_d;
          tf_d = GetFromTransferFunction (anchor + hint * 0.25);
          tf_e = GetFromTransferFunction (anchor + hint * 0.75);

          Salfa = S2left;
          S2left = h_12 * (anchor_color.w + 4.0 * tf_d.w + tf_c.w);
          S2alfa = S2left + (h_12 * (tf_c.w + 4.0 * tf_e.w + tf_b.w));
        } while (hint > hmin && abs (S2alfa - Salfa) > tol);
        hproj = std::max (hmin, hint);
      }
    }
    //Integral Interna
    /////////////////////////////////////////////////

    /////////////////////////////////////////////////
    //Integral Externa
    {
      double h = hext;
      double se1 = anchor + hint;
      //Caso o intervalo da integral externa for maior 
      // que o intervalo da integral interna
      if (hint <= hext)
      {
        h = hint;
      
        if (S2alfa == 0.0)
        {
          anchor_color = tf_b;
          anchor = anchor + h;
          F_a = glm::dvec4 (0.0);
          
          hext = std::max (hext, hproj);
          
          continue;
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

            anchor_color = tf_b;
            anchor = anchor + h;
            F_a = F_b;

            hext = std::max (hext, hproj);

            continue;
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

            anchor_color = tf_b;
            anchor = anchor + h;
            F_a = F_b;

            hext = std::max (hmin, h);
            h = std::min (hext, se1 - anchor);
          }
        }
      }

      //O passo que deve ser integrado deve ser menor ao passo
      // feito na integral interna
      do
      {
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

          anchor_color = tf_b;
          anchor = anchor + h;
          F_a = F_b;

          if (se1 <= anchor) break;

          hext = 2.0 * h;
        }
        else if (h <= hmin)
        {
          color += S2 + S2S / 15.0;
          S2alfa = h_12 * (anchor_color.w + 4.0 * tf_d.w + 2.0 * tf_c.w + 4.0 * tf_e.w + tf_b.w);
          pre_integrated = pre_integrated + (S2alfa + (S2alfa - (h_6 * (anchor_color.w + 4.0 * tf_c.w + tf_b.w))) / 15.0);

          anchor_color = tf_b;
          anchor = anchor + h;
          F_a = F_b;

          if (se1 <= anchor) break;

          hext = hmin;
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

          anchor_color = tf_b;
          anchor = anchor + h;
          F_a = F_b;

          hext = std::max (hmin, h);
        }
        h = std::min (hext, se1 - anchor);
      } while (true);
    }
    //Integral Externa
    /////////////////////////////////////////////////
  }
}
