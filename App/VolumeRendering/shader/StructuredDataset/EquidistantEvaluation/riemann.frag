#version 430

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

uniform vec3 tex_scale;
//////////////////////
//*VARIÁVEIS COMUNS*//
//////////////////////

//////////////////////////
// VARIÁVEIS ADICIONAIS //
//////////////////////////
uniform float step_size;
//////////////////////////
//*VARIÁVEIS ADICIONAIS*//
//////////////////////////

vec3 real_minpos;
vec3 real_normalized_step;

vec4 GetFromTransferFunction (float p_d)
{
  return texture(txt1D_transferfunction, texture(txt3D_volume, (real_minpos + p_d * real_normalized_step) * tex_scale).r);
}

void main(void)
{
  vec4 colorAcum = vec4(0.0);
  float alphaAcum = 1.0;

  vec3 expoint = texture(txt2D_exitpoints, gl_FragCoord.st / vec2(ScreenSizeW, ScreenSizeH)).xyz;;
  vec3 real_maxpos = expoint;
  real_minpos = EntryPoint;
  if (real_minpos == real_maxpos)
    discard;
  
  real_minpos = vec3(VolWidth*real_minpos.x, VolHeight*real_minpos.y, VolDepth*real_minpos.z);
  real_maxpos = vec3(VolWidth*real_maxpos.x, VolHeight*real_maxpos.y, VolDepth*real_maxpos.z);
  real_normalized_step = normalize(real_maxpos - real_minpos);
  
  float s0 = 0;
  float s1 = distance(real_minpos, real_maxpos);
  float s = s0;
  
  float h = step_size;

  while (s < s1)
  {
    h = min (h, s1 - s);
    
    vec4 src = GetFromTransferFunction(s + h);
    
    float d = h;
    colorAcum.x += src.x*src.w*d * alphaAcum;
    colorAcum.y += src.y*src.w*d * alphaAcum;
    colorAcum.z += src.z*src.w*d * alphaAcum;
    colorAcum.w += src.w*d       * alphaAcum;
  
    alphaAcum *= (1.0 - src.w*d);
  
    s = s + h;
  }
  FragColor = colorAcum;
}