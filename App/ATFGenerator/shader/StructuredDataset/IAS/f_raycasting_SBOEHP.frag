/*
  Step based on half subdivision intervals
*/
#version 430 compatibility

in vec3 EntryPoint;
in vec4 ExitPointCoord;

layout (location = 0) out vec4 FragColor;

//////////////////////
// VARIÁVEIS COMUNS //
//////////////////////
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
//////////////////////
//*VARIÁVEIS COMUNS*//
//////////////////////

//////////////////////////
// VARIÁVEIS ADICIONAIS //
//////////////////////////
uniform float initial_step;
uniform float tol_error;
//////////////////////////
//*VARIÁVEIS ADICIONAIS*//
//////////////////////////

vec2 ScreenSize = vec2(ScreenSizeW, ScreenSizeH);

float cte_maxstep = 8.0f;
float cte_minstep = 0.01f;
float error_boundary = 0.000001f;

float pre_integrated = 0;
float pre_alpha = 0;
vec4 color = vec4(0);

vec4 Cminpost;
float minpost;

vec3 real_normalized_step;
vec3 real_minpos;
vec3 real_maxpos;

vec4 external_aux_error;

float ivolw = GeomVolWidth  / (2.0f * VolWidth );
float ivolh = GeomVolHeight / (2.0f * VolHeight);
float ivold = GeomVolDepth  / (2.0f * VolDepth );

float svolw = GeomVolWidth  - (GeomVolWidth  / VolWidth );
float svolh = GeomVolHeight - (GeomVolHeight / VolHeight);
float svold = GeomVolDepth  - (GeomVolDepth  / VolDepth );

float MaxVec4Value (vec4 val)
{
  return max(max(val.r, val.g), max(val.b, val.a));
}

vec4 GetFromTransferFunction (float p_d)
{
  vec3 p = real_minpos + p_d * real_normalized_step;
  float value = texture(txt3D_volume, vec3(
    //ivolw +
    ((p.x / VolWidth ) * (1.f / GeomVolWidth ) 
    //* svolw
    ), 
    //ivolh +
    ((p.y / VolHeight) * (1.f / GeomVolHeight) 
    //* svolh
    ), 
    //ivold +
     ((p.z / VolDepth ) * (1.f / GeomVolDepth ) 
     //** svold
     )
  )).r; 
  return texture(txt1D_transferfunction, value);
}

float ScaleError (float error, float h0, float h)
{
  float scl_error = (error * h) / h0;
  return scl_error > error_boundary ? scl_error : error_boundary;
}

bool ColorErrorEvalFunc (vec4 a, vec4 b, float tol)
{
  tol = 15.0f * tol;

  float error_r = abs (a.x - b.x);
  float error_g = abs (a.y - b.y);
  float error_b = abs (a.z - b.z);
  float error_a = abs (a.w - b.w);

  external_aux_error = vec4 (error_r, error_g, error_b, error_a);

  return (tol >= error_r && tol >= error_g &&
  tol >= error_b && tol >= error_a);
}

vec4 ExtenalEvaluation (float p_d, vec4 C, bool computealpha)
{
  float alpha = 0.0f;
  float h = p_d - minpost;
  if (h != 0.0f)
    alpha = (h / 6.0f) * (Cminpost.w + 4.0f * GetFromTransferFunction ((minpost + p_d) / 2.0f).w + C.w);
  
  float innerint = exp (-(pre_integrated + alpha));

  if (computealpha)
    pre_alpha = pre_integrated + alpha;

  float alphachannel = C.w*innerint;
  return vec4 (
    alphachannel * C.x,
    alphachannel * C.y,
    alphachannel * C.z,
    alphachannel
    );
}

vec4 ExtenalEvaluationMiddle (float p_d, vec4 C, vec4 Cmid, bool computealpha)
{
  float alpha = 0.0f;
  float h = p_d - minpost;
  if (h != 0.0f)
    alpha = (h / 6.0f) * (Cminpost.w + 4.0f * Cmid.w + C.w);

  float innerint = exp (-(pre_integrated + alpha));

  if (computealpha)
    pre_alpha = pre_integrated + alpha;

  float alphachannel = C.w*innerint;
  return vec4 (
    alphachannel * C.x,
    alphachannel * C.y,
    alphachannel * C.z,
    alphachannel
    );
}

bool IntegrateExternalInterval(float s, float h, float tol)
{
  float a, d, c, e, b;
  a = s; b = s + h; c = (a + b) / 2.0f;
  d = (a + c) / 2.0f; e = (c + b) / 2.0f;

  vec4 gf_b = GetFromTransferFunction (b);
  vec4 gf_d = GetFromTransferFunction (d);
  vec4 gf_c = GetFromTransferFunction (c);
  
  vec4 fa = ExtenalEvaluation (a, Cminpost, false);
  vec4 fd = ExtenalEvaluation (d, gf_d, false);
  vec4 fc = ExtenalEvaluationMiddle (c, gf_c, gf_d, false);
  vec4 fe = ExtenalEvaluation (e, GetFromTransferFunction (e), false);
  vec4 fb = ExtenalEvaluationMiddle (b, gf_b, gf_c, true);
  
  vec4 S = (h / 6.0f) * (fa + 4.0f * fc + fb); 
  vec4 Sleft = (h / 12.0f) * (fa + 4.0f * fd + fc); 
  vec4 Sright = (h / 12.0f) * (fc + 4.0f * fe + fb);
  vec4 S2 = Sleft + Sright;

  if (ColorErrorEvalFunc (S, S2, tol))
  {
    color += S2 + (S2 - S) / 15.0f;
    pre_integrated = pre_alpha;

    Cminpost = gf_b;

    return true;
  }
  return false;
}

float IntegrateInternalInterval (float s, float h, float tol)
{
  float d, c, e, b;
  b = s + h; c = (s + b) / 2.0f;
  d = (s + c) / 2.0f; e = (c + b) / 2.0f;

  float fa = Cminpost.w;
  float fd = GetFromTransferFunction(d).w;
  float fc = GetFromTransferFunction(c).w;
  float fe = GetFromTransferFunction(e).w;
  float fb = GetFromTransferFunction(b).w;

  float S      = (h / 6.0f)  * (fa + 4.0f * fc + fb); 
  float Sleft  = (h / 12.0f) * (fa + 4.0f * fd + fc); 
  float Sright = (h / 12.0f) * (fc + 4.0f * fe + fb);

  return abs ((Sleft + Sright) - S);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

float hproj = 0.0f;

float InternalIntegral (float s, float h0, float error)
{
  float h = h0;
  float h_error = error;
  float exponent = (1.0f / 3.0f);

  float ierror = IntegrateInternalInterval (s, h, h_error);
  hproj = min (pow (((15.0f*h_error) / ierror), exponent) * h, cte_maxstep);

  bool done = ierror <= 15.0f*h_error;
  while (!done && hproj < h && h > cte_minstep)
  {
    h = hproj;
    h_error =  ScaleError (error, h0, h);
    ierror = IntegrateInternalInterval (s, h, h_error);
    hproj = min (pow (((15.0f*h_error) / ierror), exponent) * h, cte_maxstep);
    hproj = min (hproj, h / 2.0f);
  
    done = ierror <= 15.0f*h_error;
  }

  return h;
}

float ExternalIntegral (float s, float h0, float error)
{
  float h = h0;
  float s1 = s + h;
  float H_error = error;
  float exponent = (1.0f / 3.0f);
  
  vec4 S = vec4 (0.0f);
  vec4 gftd_b = vec4 (0.0f);

  while (s1 > s)
  {
    H_error = ScaleError (error, h0, h);
    if (!IntegrateExternalInterval (s, h, H_error))
    {
      h = max (pow (((15.0f*H_error) / MaxVec4Value (external_aux_error)), exponent) * h, cte_minstep);

      float a, c, b;
      a = s; b = s + h; c = (a + b) / 2.0f;
      vec4 gftd_c = GetFromTransferFunction (c);
      gftd_b = GetFromTransferFunction (b);
      
      vec4 fa = ExtenalEvaluation (a, Cminpost, false);
      vec4 fc = ExtenalEvaluation (c, gftd_c, false);
      vec4 fb = ExtenalEvaluationMiddle (b, gftd_b, gftd_c, true);
      pre_integrated = pre_alpha;

      S = (h / 6.0f)  * (fa + 4.0f * fc + fb); 
     
      color += S;
      Cminpost = gftd_b;
    }
    
    s = s + h;
    minpost = s;
  }

  return h;
}

float ExternalIntegralOneStep (float s, float h0, float error)
{
  float h = h0;
  float H_error = error;
  float exponent = (1.0f / 3.0f);

  while (!IntegrateExternalInterval (s, h, H_error))
  {
    float exhproj = max (pow (((15.0f*H_error) / MaxVec4Value (external_aux_error)), exponent) * h, cte_minstep);
    h = min (exhproj, h / 2.0f);
    H_error = ScaleError (error, h0, h);

    if (h <= cte_minstep)
    {
      float a, c, b;
      a = s; b = s + h; c = (a + b) / 2.0f;
      vec4 gftd_c = GetFromTransferFunction (c);
      vec4 gftd_b = GetFromTransferFunction (b);
      
      vec4 fa = ExtenalEvaluation (a, Cminpost, false);
      vec4 fc = ExtenalEvaluation (c, gftd_c, false);
      vec4 fb = ExtenalEvaluationMiddle (b, gftd_b, gftd_c, true);
      pre_integrated = pre_alpha;
      
      vec4 S = (h / 6.0f) * (fa + 4.0f * fc + fb); 
      color += S;
      
      Cminpost = gftd_b;
      break;
    }

  }
 
  return h;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void main()
{
  //Inicialização de variáveis principais:
  real_maxpos = texture(txt2D_exitpoints, gl_FragCoord.st / ScreenSize).xyz;
  real_minpos = EntryPoint;
  if (real_minpos == real_maxpos)
    discard;
    
  real_minpos = vec3(VolWidth*real_minpos.x, VolHeight*real_minpos.y, VolDepth*real_minpos.z);
  real_maxpos = vec3(VolWidth*real_maxpos.x, VolHeight*real_maxpos.y, VolDepth*real_maxpos.z);
  real_normalized_step = normalize(real_maxpos - real_minpos);

  //Init
  float s0 = 0;
  float s1 = distance(real_minpos, real_maxpos);
  float s = s0;
  float h = min (initial_step, s1 - s);

  Cminpost = GetFromTransferFunction (s);
  while (s1 > s)
  {   
    minpost = s;

    h = max (h, hproj);
    h = max (h, cte_minstep);
    h = min (h, s1 - s);
    h = min (h, cte_maxstep);

    //Calcular integral interna
    h = InternalIntegral (s, h, ScaleError (tol_error, s1 - s0, h));
    h = min (h, hproj);
    h = max (h, min (cte_minstep, s1 - s)); 
   
    //Calcular integral externa
    h = ExternalIntegralOneStep (s, h, ScaleError (tol_error, s1 - s0, h));
    //ExternalIntegral (s, h, ScaleError (tol_error, s1 - s0, h));
    
    //Increment evaluation
    s = s + h;
  }
 
  FragColor = color;
  
  //Front face
  //FragColor = vec4(EntryPoint, 1.0);

  //Back face
  //FragColor = vec4(exitPoint, 1.0);
}