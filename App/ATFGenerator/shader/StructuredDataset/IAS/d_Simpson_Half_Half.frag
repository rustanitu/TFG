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

uniform double GeomVolWidth;
uniform double GeomVolHeight;
uniform double GeomVolDepth;

uniform double initial_step;
uniform double tol_error;

float pre_integrated = 0;
vec4 color = vec4(0);

vec4 clr[5];
float v[6];
vec4 Cminpost;
float minpost;

vec3 real_normalized_step;
vec3 real_minpos;
vec3 real_maxpos;

float ivolw = GeomVolWidth  / (2.0f * VolWidth );
float ivolh = GeomVolHeight / (2.0f * VolHeight);
float ivold = GeomVolDepth  / (2.0f * VolDepth );

float svolw = GeomVolWidth  - (GeomVolWidth  / VolWidth );
float svolh = GeomVolHeight - (GeomVolHeight / VolHeight);
float svold = GeomVolDepth  - (GeomVolDepth  / VolDepth );

vec4 GetFromTransferFunction (float p_d)
{
  vec3 p = real_minpos + p_d * real_normalized_step;
  float value = texture(txt3D_volume, vec3(
    /*ivolw +*/ ((p.x / VolWidth ) * (1.f / GeomVolWidth ) /** svolw*/), 
    /*ivolh +*/ ((p.y / VolHeight) * (1.f / GeomVolHeight) /** svolh*/), 
    /*ivold +*/ ((p.z / VolDepth ) * (1.f / GeomVolDepth ) /** svold*/)
  )).r; 
  return texture(txt1D_transferfunction, value);
}

float ScaleError (float error, float h0, float h)
{
  float scl_error = (error * h) / h0;
  return scl_error > 0.000001 ? scl_error : 0.000001;
}

bool ColorErrorFunc (vec4 a, vec4 b, float tol)
{
  float error_r = abs (a.x - b.x);
  float error_g = abs (a.y - b.y);
  float error_b = abs (a.z - b.z);
  float error_a = abs (a.w - b.w);

  return (tol >= error_r && tol >= error_g &&
  tol >= error_b && tol >= error_a);
}

vec4 ExternalIntegration (vec4 C, float inner)
{
  float innerint = exp (-(pre_integrated + inner));

  float alphachannel = C.w*innerint;
  return vec4 (
    alphachannel * C.x,
    alphachannel * C.y,
    alphachannel * C.z,
    alphachannel
    );
}

vec4 aext_left_S, aext_right_S;
bool TryExternaIntegration (float s, float h, float tol)
{
  float a = s, c = (s + h) / 2.0, b = s + h;
  float d = (a + c) / 2.0, e = (c + b) / 2.0;

  vec4 fa = ExternalIntegration (clr[0], 0);
  vec4 fd = ExternalIntegration (clr[1], v[2]);
  vec4 fc = ExternalIntegration (clr[2], v[2] + v[3] + (v[2] + v[3] - v[0]) / 15.0);
  vec4 fe = ExternalIntegration (clr[3], (v[2] + v[3] + (v[2] + v[3] - v[0]) / 15.0) + v[4]);
  vec4 fb = ExternalIntegration (clr[4], (v[2] + v[3] + (v[2] + v[3] - v[0]) / 15.0) + 
                                 (v[4] + v[5] + (v[4] + v[5] - v[1]) / 15.0));

  vec4 S = (h / 6.0f) * (fa + (4.0f * fc) + fb);
  vec4 Sleft = (h / 12.0f) * (fa + (4.0f * fd) + fc);
  vec4 Sright = (h / 12.0f) * (fc + (4.0f * fe) + fb);
  vec4 S2 = Sleft + Sright;

  if (ColorErrorFunc (S, S2, tol*15.0f) || h <= 0.01)
  {
    vec4 Sret = S2 + (S2 - S) / 15.0f;
    color += Sret;
    return true;
  }

  aext_left_S = Sleft;
  aext_right_S = Sright;
  return false;
}

vec4 ExtenalEvaluation (float p_d, vec4 C)
{
  float alpha = 0.0f;
  float h = p_d - minpost;
  if (h != 0.0f)
    alpha = (h / 6.0f) * (Cminpost.w + 4.0f * GetFromTransferFunction ((minpost + p_d) / 2.0f).w + C.w);

  return ExternalIntegration (C, alpha);
}

vec4 R_Sleft = vec4(0);
vec4 R_Sright = vec4(0);
vec4 aux_ext_S = vec4(0);
bool IntegrateExternalInterval (float a, float b, float tol, vec4 ifa, vec4 ifd, vec4 ifc, vec4 ife, vec4 ifb, bool forcecomp, vec4 Spret)
{
  float h = (b - a);

  float c, d, e;
  c = (a + b) / 2.0f;
  d = (a + c) / 2.0f; e = (c + b) / 2.0f;
  vec4 fa = ExtenalEvaluation (a, ifa);
  vec4 fd = ExtenalEvaluation (d, ifd);
  vec4 fc = ExtenalEvaluation (c, ifc);
  vec4 fe = ExtenalEvaluation (e, ife);
  vec4 fb = ExtenalEvaluation (b, ifb);

  vec4 S = Spret;
  vec4 Sleft = (h / 12.0f) * (fa + 4.0f * fd + fc); 
  vec4 Sright = (h / 12.0f) * (fc + 4.0f * fe + fb);
  vec4 S2 = Sleft + Sright;

  if (ColorErrorFunc (S2, S, tol*15.0f) || forcecomp)
  {
    R_Sleft = Sleft;
    R_Sright = Sright;
    aux_ext_S = S2 + (S2 - S) / 15.0f;
    return true;
  }

  aext_left_S = Sleft;
  aext_right_S = Sright;
  return false;
}

vec4 Fleft[5];
vec4 Fright[5];
vec4 lastColor;
float Aux_AdaptiveExternalIntegration (float s, float h0, float tol)
{
  float h = h0;
  float h_2 = h / 2.0f;
  float scl_error;
    
  vec4 colorleft;
  vec4 colorright;

  float a, b, c, d, e;
  a = s; b = s + h_2; c = (a + b) / 2.0f;
  Fleft[0] = lastColor;
  Fleft[1] = GetFromTransferFunction (c);
  Fleft[2] = GetFromTransferFunction (b);

  float fdist = h;
  do
  {
    h = fdist;
    h_2 = h / 2.0;

    //set aux loop variables
    fdist /= 2.0;

    scl_error = ScaleError (tol, h0, h_2);

    a = s; b = s + h_2; c = (a + b) / 2.0;
    d = (a + c) / 2.0; e = (c + b) / 2.0;
    Fleft[4] = Fleft[2];
    Fleft[2] = Fleft[1];
    Fleft[1] = GetFromTransferFunction (d);
    Fleft[3] = GetFromTransferFunction (e);

    if (IntegrateExternalInterval (s, s + h_2, scl_error, Fleft[0], Fleft[1], Fleft[2], Fleft[3], Fleft[4], h <= 0.01, aext_left_S))
    {
      colorleft = aux_ext_S;
      break;
    }
  } while (true);

  vec4 aext_Sleft = R_Sleft;
  vec4 aext_Sright = R_Sright;

  a = s + h_2; b = s + h; c = (a + b) / 2.0;
  d = (a + c) / 2.0; e = (c + b) / 2.0;
  Fright[0] = Fleft[4];
  Fright[1] = GetFromTransferFunction (d);
  Fright[2] = GetFromTransferFunction (c);
  Fright[3] = GetFromTransferFunction (e);
  Fright[4] = GetFromTransferFunction (b);

  if (!IntegrateExternalInterval (s + h_2, s + h, scl_error, Fright[0], Fright[1], Fright[2], Fright[3], Fright[4], h <= 0.01, aext_right_S))
  {
    h /= 2.0;
    h_2 = h / 2.0;

    scl_error = ScaleError (tol, h0, h_2);

    //arrumar valores da segunda parte
    a = s + h_2; b = s + h; c = (a + b) / 2.0;
    Fright[0] = Fleft[2];
    Fright[1] = GetFromTransferFunction ((a + c) / 2.0);
    Fright[2] = Fleft[3];
    Fright[3] = GetFromTransferFunction ((c + b) / 2.0);
    Fright[4] = Fleft[4];
    //arrumar valores da primeira parte
    a = s; b = s + h_2; c = (a + b) / 2.0;
    Fleft[4] = Fleft[2];
    Fleft[2] = Fleft[1];
    Fleft[1] = GetFromTransferFunction ((a + c) / 2.0);
    Fleft[3] = GetFromTransferFunction ((c + b) / 2.0);

    IntegrateExternalInterval (s, s + h_2, scl_error, Fleft[0], Fleft[1], Fleft[2], Fleft[3], Fleft[4], true, aext_Sleft);
    colorleft = aux_ext_S;
    IntegrateExternalInterval (s + h_2, s + h, scl_error, Fright[0], Fright[1], Fright[2], Fright[3], Fright[4], true, aext_Sright);
  }
  colorright = aux_ext_S;
  color += colorleft + colorright;

  lastColor = Fright[4];

  return h;
}

void AdaptiveExternalIntegration (float s, float h, float tol)
{
  minpost = s;
  Cminpost = clr[0];
  
  float d = h;
  float s1 = s + h;
  
  lastColor = Cminpost;
  
  while (s1 > s)
    s += Aux_AdaptiveExternalIntegration (s, s1 - s, ScaleError(tol, d, abs(s1 - s)));
}

bool AlphaErrorEvalFunc (float a, float b, float tol)
{
  return tol >= abs (b - a);
}

float aux_int_S = 0; float aux_int_Sleft = 0; float aux_int_Sright = 0;
float aint_left_S, aint_right_S;
bool IntegrateInternalInterval (float a, float b, float tol, float fa, float fd, float fc, float fe, float fb, bool forcecomp, float Spre)
{
  float h = (b - a);

  float S = Spre >= 0 ? Spre : (h / 6.0) * (fa + 4.0 * fc + fb);
  float Sleft = (h / 12.0) * (fa + 4.0 * fd + fc); 
  float Sright = (h / 12.0) * (fc + 4.0 * fe + fb); 
  float S2 = Sleft + Sright;

  if (AlphaErrorEvalFunc (S2, S, tol*15.0) || forcecomp)
  {
    aux_int_Sright = Sright;
    aux_int_Sleft = Sleft;
    aux_int_S = S;
    return true;
  }

  aint_left_S = Sleft;
  aint_right_S = Sright;
  return false;
}

float AdaptiveInternalIntegration (float s, float h0, float tol)
{
  float h = h0;
  float h_2 = h / 2.0;
  float scl_error;
 
  vec4 fleft[5];
  vec4 fright[5];
  
  aint_left_S = aint_right_S = -1.f;
  
  float a, b, c, d, e;
  
  a = s; b = s + h_2; c = (a + b) / 2.0;
  fleft[0] = clr[0];
  fleft[1] = GetFromTransferFunction (c);
  fleft[2] = GetFromTransferFunction (b);

  float fdist = h;
  do
  {
    h = fdist;
    h_2 = h / 2.0; 
    
    //set aux loop variables
    fdist /= 2.0;

    scl_error = ScaleError(tol, h0, h_2);
    
    a = s; b = s + h_2; c = (a + b) / 2.0;
    d = (a + c) / 2.0; e = (c + b) / 2.0;
    fleft[4] = fleft[2];
    fleft[2] = fleft[1];
    fleft[1] = GetFromTransferFunction (d);
    fleft[3] = GetFromTransferFunction (e);

    if (IntegrateInternalInterval (s, s + h_2, scl_error, fleft[0].w, fleft[1].w, fleft[2].w, fleft[3].w, fleft[4].w, h <= 0.01, aint_left_S))
    {
      v[0] = aux_int_S;
      v[2] = aux_int_Sleft;
      v[3] = aux_int_Sright;
      break;
    }
  } while (true);

  a = s + h_2; b = s + h; c = (a + b) / 2.0;
  d = (a + c) / 2.0; e = (c + b) / 2.0;
  fright[0] = fleft[4];
  fright[1] = GetFromTransferFunction (d);
  fright[2] = GetFromTransferFunction (c);
  fright[3] = GetFromTransferFunction (e);
  fright[4] = GetFromTransferFunction (b);
  
  if (!IntegrateInternalInterval (s + h_2, s + h, scl_error, fright[0].w, fright[1].w, fright[2].w, fright[3].w, fright[4].w, h <= 0.01, aint_right_S))
  {
    aint_left_S = v[2];
    aint_right_S = v[3];

    h /= 2.0;
    h_2 = h / 2.0;

    scl_error = ScaleError(tol, h0, h_2);

    //arrumar valores da segunda parte
    a = s + h_2; b = s + h; c = (a + b) / 2.0;
    fright[0] = fleft[2];
    fright[1] = GetFromTransferFunction ((a + c) / 2.0);
    fright[2] = fleft[3];
    fright[3] = GetFromTransferFunction ((c + b) / 2.0);
    fright[4] = fleft[4];
    //arrumar valores da primeira parte
    a = s; b = s + h_2; c = (a + b) / 2.0;
    fleft[4] = fleft[2];
    fleft[2] = fleft[1];
    fleft[1] = GetFromTransferFunction ((a + c) / 2.0);
    fleft[3] = GetFromTransferFunction ((c + b) / 2.0);
    
    IntegrateInternalInterval (s, s + h_2, scl_error, fleft[0].w, fleft[1].w, fleft[2].w, fleft[3].w, fleft[4].w, true, aint_left_S);
    v[0] = aux_int_S;
    v[2] = aux_int_Sleft;
    v[3] = aux_int_Sright;
    IntegrateInternalInterval (s + h_2, s + h, scl_error, fright[0].w, fright[1].w, fright[2].w, fright[3].w, fright[4].w, true, aint_right_S);
  }
  v[1] = aux_int_S;
  v[4] = aux_int_Sleft;
  v[5] = aux_int_Sright;
  
  clr[0] = fleft[0];
  clr[1] = fleft[2];
  clr[2] = fleft[4];
  clr[3] = fright[2];
  clr[4] = fright[4];

  return h;
}

float h0 = initial_step;
void main()
{
  //Inicialização de variáveis principais:
  real_maxpos = texture(txt2D_exitpoints, gl_FragCoord.st / vec2(ScreenSizeW, ScreenSizeH)).xyz;
  real_minpos = EntryPoint;
  if (real_minpos == real_maxpos)
    discard;
  
  v[0] = 0; v[1] = 0; v[2] = 0;
  v[3] = 0; v[4] = 0; v[5] = 0;
  
  real_minpos = vec3(VolWidth*real_minpos.x, VolHeight*real_minpos.y, VolDepth*real_minpos.z);
  real_maxpos = vec3(VolWidth*real_maxpos.x, VolHeight*real_maxpos.y, VolDepth*real_maxpos.z);
  real_normalized_step = normalize(real_maxpos - real_minpos);

  float s0 = 0;
  float s1 = distance(real_minpos, real_maxpos);
  float s = s0;
 
  h0 = min (h0, s1);
  clr[0] = GetFromTransferFunction (s);
  while (s1 > s)
  {
    float h = AdaptiveInternalIntegration (s, h0, ScaleError(tol_error, s1 - s0, h0));
    float max_h_tol = ScaleError(tol_error, s1 - s0, h);
    if (!TryExternaIntegration (s, h, max_h_tol))
      AdaptiveExternalIntegration (s, h, max_h_tol);
    
    s = s + h;
    
    h = h == h0 ? 2.0 * h : h;
    h0 = min (h, s1 - s);
    h0 = min (h0, 10.0);
      
    pre_integrated += (v[2] + v[3] + (v[2] + v[3] - v[0]) / 15.0) +
                      (v[4] + v[5] + (v[4] + v[5] - v[1]) / 15.0);
    clr[0] = clr[4];
  }
  
  FragColor = color;
  
  //Front face
  //FragColor = vec4(EntryPoint, 1.0);

  //Back face
  //FragColor = vec4(exitPoint, 1.0);
}