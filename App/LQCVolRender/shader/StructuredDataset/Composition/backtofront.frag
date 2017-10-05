#version 430

in vec3 EntryPoint;
in vec4 ExitPointCoord;

layout (location = 0) out vec4 FragColor;

//////////////////////
// VARIÁVEIS COMUNS //
//////////////////////
uniform int ScreenSizeW;
uniform int ScreenSizeH;

uniform sampler2D ExitPoints;
uniform sampler3D VolumeTex;
uniform sampler1D TransferFunc;  

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
  return texture(TransferFunc, texture(VolumeTex, (real_minpos + p_d * real_normalized_step) * tex_scale).r);
}

void main(void)
{
  vec4 dst = vec4(0.0);
  vec4 src;

  vec3 expoint = texture(ExitPoints, gl_FragCoord.st / vec2(ScreenSizeW, ScreenSizeH)).xyz;;
  vec3 real_maxpos = expoint;
  real_minpos = EntryPoint;
  if (real_minpos == real_maxpos)
    discard;
  
  real_minpos = vec3(VolWidth*real_minpos.x, VolHeight*real_minpos.y, VolDepth*real_minpos.z);
  real_maxpos = vec3(VolWidth*real_maxpos.x, VolHeight*real_maxpos.y, VolDepth*real_maxpos.z);
  real_normalized_step = normalize(real_maxpos - real_minpos);
  
  float s0 = 0;
  float s1 = distance(real_minpos, real_maxpos);
  float s = s1;
  
  float h = 0.5f;

  while (s > 0)
  {
    h = min (h, s);
    
    src = GetFromTransferFunction(s);
    
    dst.rgb = src.rgb + (1 - src.a) * dst.rgb;
    dst.a   = src.a   + (1 - src.a) * dst.a;
    
    //break from the loop when alpha gets high enough
    if(dst.a >= .95f)
      break; 
  
    s = s - h;
  }
  FragColor = dst;
}