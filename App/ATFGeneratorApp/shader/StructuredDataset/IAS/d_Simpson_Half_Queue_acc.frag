#version 430 compatibility
#extension GL_ARB_gpu_shader_fp64 : enable
#extension GL_ARB_gpu_shader5 : enable

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

uniform double GeomVolWidth;
uniform double GeomVolHeight;
uniform double GeomVolDepth;

uniform double initial_step;
uniform double tol_error;

double hproj = 0;

dvec4 Cminpost;
double minpost;

double pre_integrated = 0;
dvec4 color = dvec4(0);
dvec4 f_int[5];

dvec3 real_normalized_step;
dvec3 real_minpos;
dvec3 real_maxpos;

double ivolw = GeomVolWidth  / (2.0 * VolWidth );
double ivolh = GeomVolHeight / (2.0 * VolHeight);
double ivold = GeomVolDepth  / (2.0 * VolDepth );

double svolw = GeomVolWidth  - (GeomVolWidth  / VolWidth );
double svolh = GeomVolHeight - (GeomVolHeight / VolHeight);
double svold = GeomVolDepth  - (GeomVolDepth  / VolDepth );

dvec4 GetFromTransferFunction (double p_d)
{
  dvec3 p = real_minpos + p_d * real_normalized_step;
  float value = texture(txt3D_volume, vec3(
    ((p.x / VolWidth ) * (1.0f / GeomVolWidth ) ), 
    ((p.y / VolHeight) * (1.0f / GeomVolHeight) ), 
    ((p.z / VolDepth ) * (1.0f / GeomVolDepth ))
  )).r; 
  vec4 tf1d = texture(txt1D_transferfunction, value);
  return dvec4(tf1d.x, tf1d.y, tf1d.z, tf1d.w);
}

double ScaleError (double error, double h0, double h)
{
  return (error * h) / h0;
}

struct SimpsonRec
{
  double h;
  dvec4 tf_b, tf_c;
  dvec4 F_b, F_c;
};

dvec4 ExternalEvaluationMinpost ()
{
  vec2 hpow = vec2(pre_integrated,0);
  double alphachannel = Cminpost.w*(exp (-(hpow.x)));
  return dvec4 (alphachannel * Cminpost.x, alphachannel * Cminpost.y, alphachannel * Cminpost.z, alphachannel);
}

dvec4 ExternalEvaluation (double p_d, dvec4 C, dvec4 Cmid)
{
  vec2 hpow = vec2(pre_integrated, (((p_d - minpost) / 6.0) * (Cminpost.w + 4.0 * Cmid.w + C.w)));
  double alphachannel = C.w*(exp (-(hpow.x + hpow.y)));
  return dvec4 (alphachannel * C.x, alphachannel * C.y, alphachannel * C.z, alphachannel);
}

bool ColorErrorEvalFunc (dvec4 a, dvec4 b, double tol)
{
  tol = 15.0 * tol;

  return (tol >= abs (a.x - b.x)
       && tol >= abs (a.y - b.y)
       && tol >= abs (a.z - b.z)
       && tol >= abs (a.w - b.w));
}

void QueueExternalIntegral (double s, double h0, double error)
{
  if (h0 == 0.0) return;
  dvec4 F_a = ExternalEvaluationMinpost ();
  dvec4 F_d, F_c, F_e, F_b;

  double a, d, c, e, b;
  dvec4 S, Sleft, Sright, S2;
  double Salfa, S2alfa;
  
  a = s;
  b = s + h0;
  c = (a + b) /2.0;
  d = (a + c) / 2.0;
  e = (c + b) / 2.0;

  F_d = ExternalEvaluation (d, f_int[1], GetFromTransferFunction ((minpost + d) / 2.0));
  F_c = ExternalEvaluation (c, f_int[2], f_int[1]);
  F_e = ExternalEvaluation (e, f_int[3], GetFromTransferFunction ((minpost + e) / 2.0));
  F_b = ExternalEvaluation (b, f_int[4], f_int[2]);

       S = (h0 / 6.00) * (F_a + (4.0 * F_c) + F_b);
   Sleft = (h0 / 12.0) * (F_a + (4.0 * F_d) + F_c);
  Sright = (h0 / 12.0) * (F_c + (4.0 * F_e) + F_b);
      S2 = Sleft + Sright;
  
  if (ColorErrorEvalFunc (S, S2, error) || h0 <= 0.01)
  {
    color += S2 + (S2 - S) / 15.0;
  
    Salfa = (h0 / 6.0) * (Cminpost.w + 4.0 * f_int[2].w + f_int[4].w);
    S2alfa =
      (h0 / 12.0) * (Cminpost.w + 4.0 * f_int[1].w + f_int[2].w) +
      (h0 / 12.0) * (f_int[2].w + 4.0 * f_int[3].w + f_int[4].w);
    
    pre_integrated = pre_integrated + S2alfa + (S2alfa - Salfa) / 15.0;
  
    Cminpost = f_int[4];
    minpost = s + h0;
  
    return;
  }
  
  
  //Pilha do simpson adaptativo
  SimpsonRec queue[11];
  int q_n = 0;
  dvec4 tf_d, tf_e;

  queue[q_n].h = h0 / 2.0;
  queue[q_n].tf_c = f_int[3];
  queue[q_n].tf_b = f_int[4];
  queue[q_n].F_c = ExternalEvaluation (s + (3.0 * h0 / 2.0), f_int[3], GetFromTransferFunction ((minpost + s + (3.0 * h0 / 2.0)) / 2.0));
  queue[q_n].F_b = ExternalEvaluation (s + h0, f_int[4], f_int[2]);
  q_n = q_n + 1;
  
  queue[q_n].h = h0 / 2.0;
  queue[q_n].tf_c = f_int[1];
  queue[q_n].tf_b = f_int[2];
  queue[q_n].F_c = ExternalEvaluation (s + h0 / 4.0, f_int[1], GetFromTransferFunction ((minpost + s + (h0 / 4.0)) / 2.0));
  queue[q_n].F_b = ExternalEvaluation (s + h0 / 2.0, f_int[2], f_int[1]);
  q_n = q_n + 1;
  
  while (q_n > 0)
  {
    SimpsonRec r = queue[q_n - 1];
    
    c = minpost + (r.h / 2.0);
    d = minpost + (r.h / 4.0);
    e = minpost + (3.0 * r.h / 4.0);
  
    tf_d = GetFromTransferFunction (d);
    tf_e = GetFromTransferFunction (e);
  
    F_d = ExternalEvaluation (d, GetFromTransferFunction (d), GetFromTransferFunction ((minpost + d) / 2.0));
    F_e = ExternalEvaluation (e, GetFromTransferFunction (e), GetFromTransferFunction ((minpost + e) / 2.0));
    
         S = (r.h / 6.00) * (  F_a + (4.0 * r.F_c) + r.F_b);
     Sleft = (r.h / 12.0) * (  F_a + (4.0 *   F_d) + r.F_c);
    Sright = (r.h / 12.0) * (r.F_c + (4.0 *   F_e) + r.F_b);
        S2 = Sleft + Sright;
    
    if (ColorErrorEvalFunc (S2, S, ScaleError (error, h0, r.h)) || r.h <= 0.01)
    {
      color += S2 + (S2 - S) / 15.0;
      q_n = q_n - 1;
  
      Salfa = (r.h / 6.0) * (Cminpost.w + 4.0 * r.tf_c.w + r.tf_b.w);
      S2alfa =
        (r.h / 12.0) * (Cminpost.w + 4.0 * tf_d.w + r.tf_c.w) +
        (r.h / 12.0) * (r.tf_c.w   + 4.0 * tf_e.w + r.tf_b.w);
      pre_integrated = pre_integrated + (S2alfa + (S2alfa - Salfa) / 15.0);
      
      Cminpost = r.tf_b;
      minpost = minpost + r.h;
      F_a = ExternalEvaluationMinpost ();
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
      q_n = q_n + 1;
    }
  }
}

double InternalIntervalError (double s, double h)
{
  return abs (
  (
  ((h / 12.0) * (f_int[0].w + (4.0 * f_int[1].w) + f_int[2].w)) 
  + 
  ((h / 12.0) * (f_int[2].w + (4.0 * f_int[3].w) + f_int[4].w))
  ) 
  - 
  ((h / 6.00) * (f_int[0].w + (4.0 * f_int[2].w) + f_int[4].w))
  );
}

//Se o passo for negado, não aumentar o passo para a próxima iteração
double InternalIntegral (double s, double h0, double error)
{
  double h = h0;
  double h_error = error;

  f_int[0] = Cminpost;
  f_int[1] = GetFromTransferFunction (s + (h / 4.0));
  f_int[2] = GetFromTransferFunction (s + (h / 2.0));
  f_int[3] = GetFromTransferFunction (s + (3.0*h / 4.0));
  f_int[4] = GetFromTransferFunction (s + h);

  double ierror = InternalIntervalError (s, h);

  if (ierror <= 15.0 * h_error || h <= 0.01)
    hproj = 2.0 * h;
  else
  {
    while (ierror > 15.0 * h_error)
    {
      h = h / 2.0;
    
      f_int[4] = f_int[2];
      f_int[2] = f_int[1];
      f_int[1] = GetFromTransferFunction (s + (h / 4.0));
      f_int[3] = GetFromTransferFunction (s + (3.0*h / 4.0));
    
      if (h <= 0.01)
        break;

      h_error = ScaleError (error, h0, h);
      ierror = InternalIntervalError (s, h);
    }
    hproj = h;
  }
  return h;
}

void main()
{
  vec3 rmax = texture(txt2D_exitpoints, gl_FragCoord.st / vec2(ScreenSizeW, ScreenSizeH)).xyz;
  real_maxpos = dvec3(rmax.x, rmax.y, rmax.z);
  real_minpos = dvec3(EntryPoint.x, EntryPoint.y, EntryPoint.z);
  if (real_minpos == real_maxpos)
    discard;
    
  real_minpos = dvec3(VolWidth*real_minpos.x, VolHeight*real_minpos.y, VolDepth*real_minpos.z);
  real_maxpos = dvec3(VolWidth*real_maxpos.x, VolHeight*real_maxpos.y, VolDepth*real_maxpos.z);
  real_normalized_step = normalize(real_maxpos - real_minpos);

  double s0 = 0;
  double s1 = distance(real_minpos, real_maxpos);
  double s = s0;
 
  double tol_int = tol_error;
  double tol_ext = tol_error;

  double h = min (initial_step, s1 - s0);

  Cminpost = GetFromTransferFunction (s);
  minpost = s;

  while (s1 > s)
  {
    h = max (max (h, hproj), 0.01);
    h = min (min (h, 10.0), s1 - s);

    h = InternalIntegral (s, h, ScaleError (tol_int, s1 - s0, h));
    QueueExternalIntegral (s, h, ScaleError (tol_ext, s1 - s0, h));
    s = s + h;
  }
  
  FragColor = vec4(color.x, color.y, color.z, color.w);
  
  //Front face
  //FragColor = vec4(EntryPoint, 1.0);

  //Back face
  //FragColor = vec4(exitPoint, 1.0);
}