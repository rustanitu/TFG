#version 400

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

uniform float GeomVolWidth;
uniform float GeomVolHeight;
uniform float GeomVolDepth;
//////////////////////
//*VARIÁVEIS COMUNS*//
//////////////////////

//////////////////////////
// VARIÁVEIS ADICIONAIS //
//////////////////////////
uniform float step_size;
uniform int render_mode;
//////////////////////////
//*VARIÁVEIS ADICIONAIS*//
//////////////////////////

vec2 ScreenSize = vec2(ScreenSizeW, ScreenSizeH);

vec3 real_minpos;
vec3 real_normalized_step;

float ivolw = GeomVolWidth  / (2.0f * VolWidth );
float ivolh = GeomVolHeight / (2.0f * VolHeight);
float ivold = GeomVolDepth  / (2.0f * VolDepth );

float svolw = GeomVolWidth  - (GeomVolWidth  / VolWidth );
float svolh = GeomVolHeight - (GeomVolHeight / VolHeight);
float svold = GeomVolDepth  - (GeomVolDepth  / VolDepth );

vec4 GetFromTransferFunction (float p_d)
{
  vec3 p = real_minpos + p_d * real_normalized_step;
  float value = texture(VolumeTex, vec3(
    (p.x / VolWidth ) * (1.f / GeomVolWidth ),
    (p.y / VolHeight) * (1.f / GeomVolHeight),
    (p.z / VolDepth ) * (1.f / GeomVolDepth )
  )).r;
  return texture(TransferFunc, value);
}

void main(void)
{
  vec4 colorAcum = vec4(0.0);
  float alphaAcum = 1.0;

  //Inicialização de variáveis principais:
  vec3 expoint = texture(ExitPoints, gl_FragCoord.st / ScreenSize).xyz;;
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
  if (render_mode == 0)
    FragColor = colorAcum;
  else if (render_mode == 1)
    FragColor = vec4(EntryPoint,1.0);
  else
    FragColor = vec4(expoint,1.0);
}