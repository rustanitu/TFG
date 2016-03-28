//TENTAR COLOCAR TUDO EM UMA FUNÇÃO + GETFROMTRANSFERFUNCTION

#version 430 
//#pragma optimize

in vec3 EntryPoint;
in vec4 ExitPointCoord;

layout (location = 0) out vec4 FragColor;

uniform int ScreenSizeW;
uniform int ScreenSizeH;

uniform sampler2D txt2D_exitpoints;
uniform sampler3D txt3D_volume;
uniform sampler1D txt1D_transferfunction;  

uniform int VolWidth;
uniform int VolHeight;
uniform int VolDepth;

uniform vec3 tex_scale;

uniform float initial_step;
uniform float tol_error;

float pre_integrated = 0;
vec4 anchor_color;
float anchor;

vec3 real_normalized_step;
vec3 real_minpos;

vec4 GetFromTransferFunction (float p_d)
{
  return texture(txt1D_transferfunction, texture(txt3D_volume, (real_minpos + p_d * real_normalized_step) * tex_scale).r);
}

vec4 ExternalEvaluation (float p_d, vec4 C, float Cmidw)
{
  float alphachannel = C.w*(exp (-(pre_integrated + (((p_d - anchor) / 6.0) * (anchor_color.w + 4.0 * Cmidw + C.w)))));
  return vec4 (alphachannel * C.x, alphachannel * C.y, alphachannel * C.z, alphachannel);
}

void main()
{
  //Inicialização de variáveis principais:
  vec3 real_maxpos = texture(txt2D_exitpoints, gl_FragCoord.st / vec2(ScreenSizeW, ScreenSizeH)).xyz;
  real_minpos = EntryPoint;
  if (real_minpos == real_maxpos)
    discard;
  
  vec4 color = vec4(0);
  real_minpos = vec3(VolWidth*real_minpos.x, VolHeight*real_minpos.y, VolDepth*real_minpos.z);
  real_maxpos = vec3(VolWidth*real_maxpos.x, VolHeight*real_maxpos.y, VolDepth*real_maxpos.z);
  real_normalized_step = normalize(real_maxpos - real_minpos);

  anchor = 0;
  anchor_color = texture(txt1D_transferfunction, texture(txt3D_volume, real_minpos * tex_scale).r);
  vec4 F_a = vec4 (anchor_color.w * anchor_color.x, anchor_color.w * anchor_color.y, anchor_color.w * anchor_color.z, anchor_color.w);
  
  float s1 = distance(real_minpos, real_maxpos);
  float tol_int_multiplier = tol_error / s1;
  float tol_ext_multiplier = tol_error / s1;
  
  float hmin = 0.04;
  float hmax = 8.0;
  float h = min (initial_step, s1);
  
  float hext = h;
  float hint = h;
  float hproj = h;
  
  float b, c, d, e, h_6, h_12,
    Salfa, S2alfa, S2left, tol;
  vec4 tf_d, tf_c, tf_e, tf_b,
    F_d, F_c, F_e, F_b,
    S, S2, S2S, S2Eleft;
  
  while (s1 > anchor)
  {
    //Integral Interna
    {
      hint = max (hproj, hmin);
      hint = min (min (hint, hmax), s1 - anchor);

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
          tol = tol * 0.5;
          h_12 = h_12 * 0.5;

          tf_b = tf_c;
          tf_c = tf_d;
          tf_d = GetFromTransferFunction (anchor + hint * 0.25);
          tf_e = GetFromTransferFunction (anchor + hint * 0.75);

          Salfa = S2left;
          S2left = h_12 * (anchor_color.w + 4.0 * tf_d.w + tf_c.w);
          S2alfa = S2left + (h_12 * (tf_c.w + 4.0 * tf_e.w + tf_b.w));
        } while (abs (S2alfa - Salfa) > tol && hint > hmin);
        hproj = hint;
      }     
    }
    //Integral Externa
    {
      float h = hext;
      
	    if (hint < h)
          h = hint;
      
	    tol = tol_ext_multiplier * h;
      h_6 = h / 6.0;
      h_12 = h_6 * 0.5;
      
      float s1 = anchor + hint;
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

          float S2alfa = h_12 * (anchor_color.w + 4.0 * tf_d.w + 2.0 * tf_c.w + 4.0 * tf_e.w + tf_b.w);
          pre_integrated = pre_integrated + (S2alfa + (S2alfa - h_6 * (anchor_color.w + 4.0 * tf_c.w + tf_b.w)) / 15.0);

          anchor_color = tf_b;
          anchor = anchor + h;
          F_a = F_b;
          
          if (s1 <= anchor) break;
       
          float h_2 = 2.0 * h;
          float s1_anchor = s1 - anchor;

          hext = h_2;
          h = min (h_2, s1_anchor);

          h_6 = h / 6.0;
          h_12 = h_6 * 0.5;
          tol = tol_ext_multiplier * h;
        }
        else
        {
          do
          {
            h = h * 0.5;
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
          } while (!((tol >= abs (S2S.w) && tol >= abs (S2S.x) && tol >= abs (S2S.y) && tol >= abs (S2S.z)) || h <= hmin));

          color += S2 + S2S / 15.0;

          h_6 = h_12 * 2.0;

          float S2alfa = h_12 * (anchor_color.w + 4.0 * tf_d.w + 2.0 * tf_c.w + 4.0 * tf_e.w + tf_b.w);
          pre_integrated = pre_integrated + (S2alfa + (S2alfa - h_6 * (anchor_color.w + 4.0 * tf_c.w + tf_b.w)) / 15.0);

          anchor_color = tf_b;
          anchor = anchor + h;
          F_a = F_b;
          hext = h;
        }
      }
    }
  }
  FragColor = color;
}