/*
  Step based on half subdivision intervals
*/
#version 430 compatibility

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

float hproj = 0;

float pre_integrated = 0;
vec4 color = vec4(0);
vec4 Cminpost;
float minpost;
vec4 f_int[5];

vec3 real_normalized_step;
vec3 real_minpos;
vec3 real_maxpos;

vec4 GetFromTransferFunction (float p_d)
{
  return texture(txt1D_transferfunction, texture(txt3D_volume, (real_minpos + p_d * real_normalized_step) * tex_scale).r);
}

float ScaleError (float error, float h0, float h)
{
  float scl_error = (error * h) / h0;
  return scl_error > 0.000001 ? scl_error : 0.000001;
}

struct SimpsonRec
{
  float h;
  vec4 tf_b, tf_c;
  vec4 F_b, F_c;
};

vec4 ExternalEvaluationMinpost ()
{
  float alphachannel = Cminpost.w*(exp (-pre_integrated));
  return vec4 (alphachannel * Cminpost.x, alphachannel * Cminpost.y, alphachannel * Cminpost.z, alphachannel);
}

vec4 ExternalEvaluation (float p_d, vec4 C, vec4 Cmid)
{
  float alphachannel = C.w*(exp (-(pre_integrated + (((p_d - minpost) / 6.0) * (Cminpost.w + 4.0 * Cmid.w + C.w)))));
  return vec4 (alphachannel * C.x, alphachannel * C.y, alphachannel * C.z, alphachannel);
}

bool ColorErrorEvalFunc (vec4 a, vec4 b, float tol)
{
  tol = 15.0 * tol;

  return (tol >= abs (a.x - b.x)
       && tol >= abs (a.y - b.y)
       && tol >= abs (a.z - b.z)
       && tol >= abs (a.w - b.w));
}

void QueueExternalIntegral (float s, float h0, float error)
{
  if (h0 == 0.0) return;
  vec4 F_a = ExternalEvaluationMinpost ();
  vec4 F_d, F_c, F_e, F_b;

  float a, d, c, e, b;
  vec4 S, Sleft, Sright, S2;
  float Salfa, S2alfa;
  
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
  
  if (ColorErrorEvalFunc (S, S2, error) || h0 <= 0.04)
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
  vec4 tf_d, tf_e;

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
    
    if (ColorErrorEvalFunc (S2, S, ScaleError (error, h0, r.h)) || r.h <= 0.04)
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

float InternalIntervalError (float s, float h)
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
float InternalIntegral (float s, float h0, float error)
{
  float h = h0;
  float h_error = error;

  f_int[0] = Cminpost;
  f_int[1] = GetFromTransferFunction (s + (h / 4.0));
  f_int[2] = GetFromTransferFunction (s + (h / 2.0));
  f_int[3] = GetFromTransferFunction (s + (3.0*h / 4.0));
  f_int[4] = GetFromTransferFunction (s + h);

  float ierror = InternalIntervalError (s, h);

  if (ierror <= 15.0 * h_error || h <= 0.04)
    //if (pow (15.0*h_error / ierror, (1.0 / 3.0)) >= 2.0)
      hproj = 2.0 * h;
    //else
    //  hproj = h;
    //hproj = pow (15.0*h_error / ierror, (1.0 / 3.0)) * h;
  else
  {
    while (ierror > 15.0 * h_error)
    {
      h = h / 2.0;
    
      f_int[4] = f_int[2];
      f_int[2] = f_int[1];
      f_int[1] = GetFromTransferFunction (s + (h / 4.0));
      f_int[3] = GetFromTransferFunction (s + (3.0*h / 4.0));
    
      if (h <= 0.04)
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
  //Inicialização de variáveis principais:
  real_maxpos = texture(txt2D_exitpoints, gl_FragCoord.st / vec2(ScreenSizeW, ScreenSizeH)).xyz;
  real_minpos = EntryPoint;
  if (real_minpos == real_maxpos)
    discard;
  
  real_minpos = vec3(VolWidth*real_minpos.x, VolHeight*real_minpos.y, VolDepth*real_minpos.z);
  real_maxpos = vec3(VolWidth*real_maxpos.x, VolHeight*real_maxpos.y, VolDepth*real_maxpos.z);
  real_normalized_step = normalize(real_maxpos - real_minpos);

  float s1 = distance(real_minpos, real_maxpos);
  float s = 0;
 
  float tol_int = tol_error / 15.0;
  float tol_ext = tol_error / 15.0;

  float h = min (initial_step, s1);

  Cminpost = GetFromTransferFunction (s);
  minpost = s;

  while (s1 > s)
  {
    h = max (max (h, hproj), 0.04);
    h = min (min (h, 10.0), s1 - s);

    h = InternalIntegral (s, h, ScaleError (tol_int, s1, h));

    if (f_int[0].w + f_int[1].w + f_int[2].w + f_int[3].w + f_int[4].w == 0.0)
    {
      Cminpost = f_int[4];
      minpost = s + h;
    }
    else
      QueueExternalIntegral (s, h, ScaleError (tol_ext, s1, h));

    s = s + h;
  }
  
  FragColor = color;
  
  //Front face
  //FragColor = vec4(EntryPoint, 1.0);

  //Back face
  //FragColor = vec4(exitPoint, 1.0);
}