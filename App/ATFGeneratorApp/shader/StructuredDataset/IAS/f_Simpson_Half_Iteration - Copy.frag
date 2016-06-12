//https://www.opengl.org/wiki/GLSL_Optimizations
//https://code.google.com/p/glsl-unit/wiki/UsingTheCompiler
#version 430 

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

uniform float GeomVolWidth;
uniform float GeomVolHeight;
uniform float GeomVolDepth;

uniform float initial_step;
uniform float tol_error;

float hproj = 0;

float pre_integrated = 0;

vec4 anchor_color;
float anchor;

vec4 f_int[4];
vec4 color = vec4(0);

vec3 real_normalized_step;
vec3 real_minpos;

float scl_x = GeomVolWidth / VolWidth;
float scl_y = GeomVolHeight / VolHeight;
float scl_z = GeomVolDepth / VolDepth;
vec4 GetFromTransferFunction (float p_d)
{
  vec3 p = real_minpos + p_d * real_normalized_step;
  return texture(txt1D_transferfunction, texture(txt3D_volume,
    vec3(p.x * scl_x, p.y * scl_y, p.z * scl_z)).r);
}

vec4 ExternalEvaluationMinpost ()
{
  float alphachannel = anchor_color.w*(exp (-pre_integrated));
  return vec4 (alphachannel * anchor_color.x, alphachannel * anchor_color.y, alphachannel * anchor_color.z, alphachannel);
}

vec4 ExternalEvaluation (float p_d, vec4 C, float Cmidw)
{
  float alphachannel = C.w*(exp (-(pre_integrated + (((p_d - anchor) / 6.0) * (anchor_color.w + 4.0 * Cmidw + C.w)))));
  return vec4 (alphachannel * C.x, alphachannel * C.y, alphachannel * C.z, alphachannel);
}

bool ColorErrorEvalFunc (vec4 a, vec4 b, float tol)
{
  return (tol >= abs (a.w - b.w)
       && tol >= abs (a.x - b.x)
       && tol >= abs (a.y - b.y)
       && tol >= abs (a.z - b.z));
}

void ExternalIntegral (float delta_S, float tol)
{
  vec4 F_a = ExternalEvaluationMinpost ();
  vec4 F_d, F_c, F_b;
  
  float S2alfa;
  vec4 S, S2;
  float b, c, d, e;
  vec4 tf_d, tf_c, tf_e, tf_b;
  
  float s1 = anchor + delta_S;
  float h = delta_S;
   
  b = anchor + h;
  c = (anchor + b) / 2.0;
  d = (anchor + c) / 2.0;
  e = (c + b) / 2.0;
  
  F_d = ExternalEvaluation (d, f_int[0], GetFromTransferFunction ((anchor + d) / 2.0).w);
  F_c = ExternalEvaluation (c, f_int[1], f_int[0].w);
  F_b = ExternalEvaluation (b, f_int[3], f_int[1].w);

   S = (h / 6.00) * (F_a + (4.0 * F_c) + F_b);
  S2 = (h / 12.0) * ((F_a + (4.0 * F_d) + F_c) + (F_c + (4.0 * ExternalEvaluation (e, f_int[2], GetFromTransferFunction ((anchor + e) / 2.0).w)) + F_b));
  
  if (ColorErrorEvalFunc (S2, S, tol) || h <= 0.04)
  {
    color += S2 + (S2 - S) / 15.0;
  
    S2alfa = (h / 12.0) * ((anchor_color.w + 4.0 * f_int[0].w + f_int[1].w) + (f_int[1].w + 4.0 * f_int[2].w + f_int[3].w));
    pre_integrated = pre_integrated + S2alfa + (S2alfa - (h / 6.0) * (anchor_color.w + 4.0 * f_int[1].w + f_int[3].w)) / 15.0;
    
    anchor_color = f_int[3];
    anchor = anchor + h;
  
    return;
  }

  h = 0.5 * h;
  tol = 0.5 * tol;
  
  while (s1 > anchor)
  {
    b = anchor + h; c = (anchor + b) / 2.0;
    d = (anchor + c) / 2.0; e = (c + b) / 2.0;
     
    tf_d = GetFromTransferFunction (d);
    tf_c = GetFromTransferFunction (c);
    tf_e = GetFromTransferFunction (e);
    tf_b = GetFromTransferFunction (b);

    F_d = ExternalEvaluation (d, tf_d, GetFromTransferFunction ((anchor + d) / 2.0).w);
    F_c = ExternalEvaluation (c, tf_c, tf_d.w);
    F_b = ExternalEvaluation (b, tf_b, tf_c.w);
       
     S = (h / 6.00) * (F_a + (4.0 * F_c) + F_b);
    S2 = (h / 12.0) * ((F_a + (4.0 * F_d) + F_c) + (F_c + (4.0 * ExternalEvaluation (e, tf_e, GetFromTransferFunction ((anchor + e) / 2.0).w)) + F_b));
    
    if (ColorErrorEvalFunc (S2, S, tol) || h <= 0.04)
    {
      color += S2 + (S2 - S) / 15.0;
  
      S2alfa = (h / 12.0) * ((anchor_color.w + 4.0 * tf_d.w + tf_c.w) + (tf_c.w + 4.0 * tf_e.w + tf_b.w));
      pre_integrated = pre_integrated + (S2alfa + (S2alfa - 
      ((h / 6.0) * (anchor_color.w + 4.0 * tf_c.w + tf_b.w))
      ) / 15.0);
      
      anchor_color = tf_b;
      anchor = anchor + h;

      h = min (h * 2.0, s1 - anchor);
    }
    else
    {
      do
      {
        h = h / 2.0;
        tol = tol / 2.0;

        tf_b = tf_c; 
        tf_c = tf_d; 
        
        d = h * 0.25 + anchor;
        e = h * 0.75 + anchor;

        tf_d = GetFromTransferFunction (d);
        tf_e = GetFromTransferFunction (e);

        F_b = F_c;
        F_c = F_d;
        
        F_d = ExternalEvaluation (d, tf_d, GetFromTransferFunction ((anchor + d) / 2.0).w);
        
         S = (h / 6.00) * (F_a + (4.0 * F_c) + F_b);
        S2 = (h / 12.0) * ((F_a + (4.0 * F_d) + F_c) + (F_c + (4.0 * ExternalEvaluation (e, tf_e, GetFromTransferFunction ((anchor + e) / 2.0).w)) + F_b));

      } while (!(ColorErrorEvalFunc (S2, S, tol) || h <= 0.04));
      
      color += S2 + (S2 - S) / 15.0;
  
      S2alfa = (h / 12.0) * ((anchor_color.w + 4.0 * tf_d.w + tf_c.w) + (tf_c.w + 4.0 * tf_e.w + tf_b.w));
      pre_integrated = pre_integrated + (S2alfa + (S2alfa - ((h / 6.0) * (anchor_color.w + 4.0 * tf_c.w + tf_b.w))) / 15.0);
      
      anchor_color = tf_b;
      anchor = anchor + h;
    }
  }
}


float InternalIntervalError (float h)
{
  return abs (
  (((h / 12.0) * (anchor_color.w + (4.0 * f_int[0].w) + f_int[1].w)) + 
  ((h / 12.0) * (f_int[1].w + (4.0 * f_int[2].w) + f_int[3].w))) 
  - 
  ((h / 6.00) * (anchor_color.w + (4.0 * f_int[1].w) + f_int[3].w)));
}

float InternalIntegral (float h0, float error)
{
  float h = h0;
  float h_error = error;
  
  f_int[0] = GetFromTransferFunction (anchor + h / 4.0);
  f_int[1] = GetFromTransferFunction (anchor + h / 2.0);
  f_int[2] = GetFromTransferFunction (anchor + h * 0.75);
  f_int[3] = GetFromTransferFunction (anchor + h);
  
  float ierror = InternalIntervalError (h);

  if (ierror <= h_error || h <= 0.04)
    hproj = 2.0 * h;
  else
  {
    do
    {
      h = h / 2.0;
    
      f_int[3] = f_int[1];
      f_int[1] = f_int[0];
      f_int[0] = GetFromTransferFunction (anchor + h * 0.25);
      f_int[2] = GetFromTransferFunction (anchor + h * 0.75);
    
      if (h <= 0.04)
        break;

      h_error = h_error / 2.0;
      ierror = InternalIntervalError (h);

    } while (ierror > h_error);
    hproj = h;
  }
  return h;
}

void main()
{
  vec3 real_maxpos = texture(txt2D_exitpoints, gl_FragCoord.st / vec2(ScreenSizeW, ScreenSizeH)).xyz;
  real_minpos = EntryPoint;
  if (real_minpos == real_maxpos)
    discard;
  
  real_minpos = vec3(VolWidth*real_minpos.x, VolHeight*real_minpos.y, VolDepth*real_minpos.z);
  real_maxpos = vec3(VolWidth*real_maxpos.x, VolHeight*real_maxpos.y, VolDepth*real_maxpos.z);
  real_normalized_step = normalize(real_maxpos - real_minpos);

  float s1 = distance(real_minpos, real_maxpos);

  float h = min (initial_step, s1);

  anchor = 0;
  anchor_color = GetFromTransferFunction (anchor);
  
  while (s1 > anchor)
  {
    if (s1 - anchor <= 0.04)
    {
      ExternalIntegral (s1 - anchor, 1.0);
      break;
    }

    h = max (max (h, hproj), 0.04);
    h = min (min (h, 10.0), s1 - anchor);
   
    h = InternalIntegral (h, (tol_error * h) / s1);
   
    if (anchor_color.w + f_int[0].w + f_int[1].w + f_int[2].w + f_int[3].w == 0.0)
    {
      anchor_color = f_int[3];
      anchor = anchor + h;
    }
    else
      ExternalIntegral (h, (tol_error * h) / s1);
  }
  
  FragColor = color;
}