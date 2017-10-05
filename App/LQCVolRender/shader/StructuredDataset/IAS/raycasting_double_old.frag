//https://www.opengl.org/wiki/Common_Mistakes
#version 430 compatibility
#extension GL_ARB_shader_precision : require
#extension GL_ARB_gpu_shader_fp64 : enable

//Provavelmente mais lento em double do que em float

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

uniform double GeomVolWidth;
uniform double GeomVolHeight;
uniform double GeomVolDepth;
//////////////////////
//*VARIÁVEIS COMUNS*//
//////////////////////

//////////////////////////
// VARIÁVEIS ADICIONAIS //
//////////////////////////
uniform double initial_step;
uniform double tol_error;
//////////////////////////
//*VARIÁVEIS ADICIONAIS*//
//////////////////////////

vec2 ScreenSize = vec2(ScreenSizeW, ScreenSizeH);

double maxstepdist = 
//0.0078125
0.5
;
int maxstep = 10;
double error_boundary = 
//0.000001
//0.00001
0.01
;

double pre_integrated = 0;
dvec4 color = dvec4(0);

dvec4 aux_Clr[5];
double alpha_values[6];
dvec4 Cminpost;
double minpost;

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
    /*ivolw + */((p.x / VolWidth ) * (1.0f / GeomVolWidth ) /** svolw*/), 
    /*ivolh + */((p.y / VolHeight) * (1.0f / GeomVolHeight) /** svolh*/), 
    /*ivold + */((p.z / VolDepth ) * (1.0f / GeomVolDepth ) /** svold*/)
  )).r; 
  vec4 tf1d = texture(txt1D_transferfunction, value);
  return dvec4(tf1d.x, tf1d.y, tf1d.z, tf1d.w);
}

double ScaleError (double error, double h0, double h)
{
  double scl_error = (error * h) / h0;
  return scl_error > error_boundary ? scl_error : error_boundary;
}

bool ColorErrorFunc (dvec4 a, dvec4 b, double tol)
{
  double error_r = abs (a.x - b.x);
  double error_g = abs (a.y - b.y);
  double error_b = abs (a.z - b.z);
  double error_a = abs (a.w - b.w);

  return (error_r <= tol && error_g <= tol && error_b <= tol && error_a <= tol);
}

dvec4 ExternalIntegration (dvec4 C, double inner)
{
  vec2 inv = vec2(pre_integrated, inner);
  float innerint = exp (-(inv.x + inv.y));

  double alphachannel = C.w*innerint;
  return dvec4 (
    alphachannel * C.x,
    alphachannel * C.y,
    alphachannel * C.z,
    alphachannel
    );
}

bool TryExternaIntegration (double s, double h, double tol)
{
  double a = s, c = (s + h) / 2.0, b = s + h;
  double d = (a + c) / 2.0, e = (c + b) / 2.0;

  dvec4 fa = ExternalIntegration (aux_Clr[0], 0);
  dvec4 fd = ExternalIntegration (aux_Clr[1], alpha_values[2]);
  dvec4 fc = ExternalIntegration (aux_Clr[2],
   alpha_values[2] + alpha_values[3] + (alpha_values[2] +
   alpha_values[3] - alpha_values[0]) / 15.0);
  dvec4 fe = ExternalIntegration (aux_Clr[3], 
    (alpha_values[2] + alpha_values[3] + (alpha_values[2] + alpha_values[3]
     - alpha_values[0]) / 15.0) + alpha_values[4]);
  dvec4 fb = ExternalIntegration (aux_Clr[4],
   (alpha_values[2] + alpha_values[3] + (alpha_values[2] 
   + alpha_values[3] - alpha_values[0]) / 15.0) + 
   (alpha_values[4] + alpha_values[5] + (alpha_values[4] 
   + alpha_values[5] - alpha_values[1]) / 15.0));

  dvec4 S = (h / 6.0) * (fa + (4.0 * fc) + fb);
  dvec4 Sleft = (h / 12.0) * (fa + (4.0 * fd) + fc);
  dvec4 Sright = (h / 12.0) * (fc + (4.0 * fe) + fb);
  dvec4 S2 = Sleft + Sright;

  if (ColorErrorFunc (S, S2, tol*15.0))
  {
    dvec4 Sret = S2 + (S2 - S) / 15.0;
    color += Sret;
    return true;
  }
  return false;
}

dvec4 ExtenalEvaluation (double p_d, dvec4 C)
{
  //C = GetFromTransferFunction (p_d);
  
  double alpha = ((p_d - minpost) / 6.0) *
    (Cminpost.w +
    4.0 * GetFromTransferFunction ((minpost + p_d) / 2.0).w +
    C.w);
  
  vec2 expinv = vec2(pre_integrated, alpha);
  double innerint = exp (-(expinv.x + expinv.y));
  
  double alphachannel = C.w*innerint;
  return dvec4 (
    alphachannel * C.x,
    alphachannel * C.y,
    alphachannel * C.z,
    alphachannel
    );
}

dvec4 IEI_pS = dvec4(0);
bool IntegrateExternalInterval (double a, double b, double tol, dvec4 ifa, dvec4 ifd, dvec4 ifc, dvec4 ife, dvec4 ifb, bool forcecomp)
{
  double h = (b - a);

  double c, d, e;
  c = (a + b) / 2.0;
  d = (a + c) / 2.0; e = (c + b) / 2.0;
  dvec4 fa = ExtenalEvaluation (a, ifa);
  dvec4 fd = ExtenalEvaluation (d, ifd);
  dvec4 fc = ExtenalEvaluation (c, ifc);
  dvec4 fe = ExtenalEvaluation (e, ife);
  dvec4 fb = ExtenalEvaluation (b, ifb);

  dvec4 S = (h / 6.0) * (fa + 4.0 * fc + fb);
  dvec4 Sleft = (h / 12.0) * (fa + 4.0 * fd + fc); 
  dvec4 Sright = (h / 12.0) * (fc + 4.0 * fe + fb);
  dvec4 S2 = Sleft + Sright;

  if (ColorErrorFunc (S2, S, tol*15.0) || forcecomp)
  {
    IEI_pS = S2 + (S2 - S) / 15.0;
    return true;
  }
  return false;
}

dvec4 IEI_fleft[5];
dvec4 IEI_fright[5];
dvec4 lastColor;
void ForceExternalIntegration (double s, double h0, double tol)
{
  double h = h0;
  double h_2 = h / 2.0;
  double scl_error, a, b, c, d, e;
  dvec4 colorleft;
  dvec4 colorright;

  scl_error = ScaleError (tol, h0, h_2);
  //arrumar valores da primeira parte
  a = s; b = s + h_2; c = (a + b) / 2.0;
  IntegrateExternalInterval (s, s + h_2, scl_error, 
    lastColor,
    GetFromTransferFunction((a + c) / 2.0),
    GetFromTransferFunction(c),
    GetFromTransferFunction((c + b) / 2.0), 
    GetFromTransferFunction(b), true);
  colorleft = IEI_pS;

    //arrumar valores da segunda parte
  a = s + h_2; b = s + h; c = (a + b) / 2.0;
  IntegrateExternalInterval (s + h_2, s + h, scl_error,
    GetFromTransferFunction(a),
    GetFromTransferFunction((a + c) / 2.0),
    GetFromTransferFunction(c),
    GetFromTransferFunction((c + b) / 2.0), 
    GetFromTransferFunction(b), true);
  colorright = IEI_pS;
  
  color += colorleft + colorright;
}

double IterativeExternalIntegration (double s, double h0, double tol)
{
  double h = h0;
  double h_2 = h / 2.0;
  double scl_error;
    
  dvec4 colorleft;
  dvec4 colorright;

  double a, b, c, d, e;
  a = s; b = s + h_2; c = (a + b) / 2.0;
  IEI_fleft[0] = lastColor;
  IEI_fleft[1] = GetFromTransferFunction (c);
  IEI_fleft[2] = GetFromTransferFunction (b);
  
  double fdist = h;
  do
  {
    h = fdist;
    h_2 = h / 2.0;

    //set aux loop variables
    fdist /= 2.0;

    scl_error = ScaleError (tol, h0, h_2);

    a = s; b = s + h_2; c = (a + b) / 2.0;
    d = (a + c) / 2.0; e = (c + b) / 2.0;
    IEI_fleft[4] = IEI_fleft[2];
    IEI_fleft[2] = IEI_fleft[1];
    IEI_fleft[1] = GetFromTransferFunction (d);
    IEI_fleft[3] = GetFromTransferFunction (e);

    if (h <= maxstepdist)
    {
      IntegrateExternalInterval (s, s + h_2, scl_error, IEI_fleft[0], IEI_fleft[1], IEI_fleft[2], IEI_fleft[3], IEI_fleft[4], true);
      colorleft = IEI_pS;
      break;
    }

    if (IntegrateExternalInterval (s, s + h_2, scl_error, IEI_fleft[0], IEI_fleft[1], IEI_fleft[2], IEI_fleft[3], IEI_fleft[4], false))
    {
      colorleft = IEI_pS;
      break;
    }
    
  } while (h > maxstepdist);
  
  a = s + h_2; b = s + h; c = (a + b) / 2.0;
  d = (a + c) / 2.0; e = (c + b) / 2.0;
  IEI_fright[0] = IEI_fleft[4];
  IEI_fright[1] = GetFromTransferFunction (d);
  IEI_fright[2] = GetFromTransferFunction (c);
  IEI_fright[3] = GetFromTransferFunction (e);
  IEI_fright[4] = GetFromTransferFunction (b);

  if (!IntegrateExternalInterval (s + h_2, s + h, scl_error, IEI_fright[0], IEI_fright[1], IEI_fright[2], IEI_fright[3], IEI_fright[4], false))
  {
    h /= 2.0;
    h_2 = h / 2.0;

    scl_error = ScaleError (tol, h0, h_2);

    //arrumar valores da segunda parte
    a = s + h_2; b = s + h; c = (a + b) / 2.0;
    IEI_fright[0] = IEI_fleft[2];
    IEI_fright[1] = GetFromTransferFunction ((a + c) / 2.0);
    IEI_fright[2] = IEI_fleft[3];
    IEI_fright[3] = GetFromTransferFunction ((c + b) / 2.0);
    IEI_fright[4] = IEI_fleft[4];
    //arrumar valores da primeira parte
    a = s; b = s + h_2; c = (a + b) / 2.0;
    IEI_fleft[4] = IEI_fleft[2];
    IEI_fleft[2] = IEI_fleft[1];
    IEI_fleft[1] = GetFromTransferFunction ((a + c) / 2.0);
    IEI_fleft[3] = GetFromTransferFunction ((c + b) / 2.0);

    IntegrateExternalInterval (s, s + h_2, scl_error, IEI_fleft[0], IEI_fleft[1], IEI_fleft[2], IEI_fleft[3], IEI_fleft[4], true);
    colorleft = IEI_pS;
    IntegrateExternalInterval (s + h_2, s + h, scl_error, IEI_fright[0], IEI_fright[1], IEI_fright[2], IEI_fright[3], IEI_fright[4], true);
  }
  colorright = IEI_pS;

  color += colorleft + colorright;

  lastColor = IEI_fright[4];
  
  return h;
}

void AdaptiveExternalIntegration (double s, double h, double tol)
{
  double d = h;
  double s1 = s + h;
  lastColor = Cminpost;
  while (s < s1)
  {
    s += IterativeExternalIntegration (s, s1 - s, ScaleError(tol, d, abs(s1 - s)));
  }
}

bool AlphaErrorEvalFunc (double a, double b, double tol)
{
  return abs (b - a) <= tol;
}

double III_pS = 0; double III_pSleft = 0; double III_pSright = 0;
bool IntegrateInternalInterval (double a, double b, double tol, double fa, double fd, double fc, double fe, double fb, bool forcecomp)
{
  double h = (b - a);

  double S =  (h / 6.0) * (fa + 4.0 * fc + fb);
  double Sleft = (h / 12.0) * (fa + 4.0 * fd + fc); 
  double Sright = (h / 12.0) * (fc + 4.0 * fe + fb); 
  double S2 = Sleft + Sright;

  if (AlphaErrorEvalFunc (S2, S, tol*15.0) || forcecomp)
  {
    III_pSright = Sright;
    III_pSleft = Sleft;
    III_pS = S;
    return true;
  }
  return false;
}

double AdaptiveInternalIntegration (double s, double h0, double tol)
{
  double h = h0;
  double h_2 = h / 2.0;
  double scl_error;
 
  dvec4 fleft[5];
  dvec4 fright[5];
  
  double a, b, c, d, e;
  
  a = s; b = s + h_2; c = (a + b) / 2.0;
  fleft[0] = aux_Clr[0];
  fleft[1] = GetFromTransferFunction (c);
  fleft[2] = GetFromTransferFunction (b);

  double fdist = h;
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

    if (IntegrateInternalInterval (s, s + h_2, scl_error, fleft[0].w, fleft[1].w, fleft[2].w, fleft[3].w, fleft[4].w, false))
    {
      alpha_values[0] = III_pS;
      alpha_values[2] = III_pSleft;
      alpha_values[3] = III_pSright;
      break;
    }
  } while (h > maxstepdist);
  
  a = s + h_2; b = s + h; c = (a + b) / 2.0;
  d = (a + c) / 2.0; e = (c + b) / 2.0;
  fright[0] = fleft[4];
  fright[1] = GetFromTransferFunction (d);
  fright[2] = GetFromTransferFunction (c);
  fright[3] = GetFromTransferFunction (e);
  fright[4] = GetFromTransferFunction (b);
  
  if (!IntegrateInternalInterval (s + h_2, s + h, scl_error, fright[0].w, fright[1].w, fright[2].w, fright[3].w, fright[4].w, false))
  {
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
    
    IntegrateInternalInterval (s, s + h_2, scl_error, fleft[0].w, fleft[1].w, fleft[2].w, fleft[3].w, fleft[4].w, true);
    alpha_values[0] = III_pS;
    alpha_values[2] = III_pSleft;
    alpha_values[3] = III_pSright;
    IntegrateInternalInterval (s + h_2, s + h, scl_error, fright[0].w, fright[1].w, fright[2].w, fright[3].w, fright[4].w, true);
  }
  alpha_values[1] = III_pS;
  alpha_values[4] = III_pSleft;
  alpha_values[5] = III_pSright;
  
  aux_Clr[0] = fleft[0];
  aux_Clr[1] = fleft[2];
  aux_Clr[2] = fleft[4];
  aux_Clr[3] = fright[2];
  aux_Clr[4] = fright[4];
  
  //IEI_fright[0] = fright[0];
  //IEI_fright[1] = fright[1];
  //IEI_fright[2] = fright[2];
  //IEI_fright[3] = fright[3];
  //IEI_fright[4] = fright[4];
  //
  //IEI_fleft[0] = fleft[0];
  //IEI_fleft[1] = fleft[1];
  //IEI_fleft[2] = fleft[2];
  //IEI_fleft[3] = fleft[3];
  //IEI_fleft[4] = fleft[4];

  return h;
}

double h0 = initial_step;
void main()
{
  //Inicialização de variáveis principais:double
  vec3 rmax = texture(txt2D_exitpoints, gl_FragCoord.st / ScreenSize).xyz;
  real_maxpos = dvec3(rmax.x, rmax.y, rmax.z);
  real_minpos = dvec3(EntryPoint.x, EntryPoint.y, EntryPoint.z);
  if (real_minpos == real_maxpos)
    discard;
  
  alpha_values[0] = 0; alpha_values[1] = 0; alpha_values[2] = 0;
  alpha_values[3] = 0; alpha_values[4] = 0; alpha_values[5] = 0;
  
  real_minpos = dvec3(VolWidth*real_minpos.x, VolHeight*real_minpos.y, VolDepth*real_minpos.z);
  real_maxpos = dvec3(VolWidth*real_maxpos.x, VolHeight*real_maxpos.y, VolDepth*real_maxpos.z);
  real_normalized_step = normalize(real_maxpos - real_minpos);
  
  double s0 = 0;
  double s1 = distance(real_minpos, real_maxpos);
  double s = s0;
 
  h0 = min (h0, s1);
  aux_Clr[0] = GetFromTransferFunction (s);
  while (s < s1)
  {
    double h = AdaptiveInternalIntegration (s, h0, ScaleError(tol_error, s1 - s0, h0));
    if (!TryExternaIntegration (s, h, ScaleError(tol_error, s1 - s0, h) ))
    {
      minpost = s;
      Cminpost = aux_Clr[0];
      
    //  AdaptiveExternalIntegration (s, h, ScaleError(tol_error, s1 - s0, h));
    }
    s = s + h;
    
    h = h == h0 ? 2.0 * h : h;
    h0 = min (h, s1 - s);
    h0 = min (h0, maxstep);
    
    pre_integrated += (alpha_values[2] + alpha_values[3] + (alpha_values[2] + alpha_values[3] - alpha_values[0]) / 15.0) +
                      (alpha_values[4] + alpha_values[5] + (alpha_values[4] + alpha_values[5] - alpha_values[1]) / 15.0);
    aux_Clr[0] = aux_Clr[4];
  }
  
  FragColor = vec4(color.x, color.y, color.z, color.w);
  
  //Front face
  //FragColor = vec4(EntryPoint, 1.0);

  //Back face
  //FragColor = vec4(exitPoint, 1.0);
}