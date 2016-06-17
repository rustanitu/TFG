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

float GetFromVolume (float p_d)
{
  return texture(VolumeTex, (real_minpos + p_d * real_normalized_step) * tex_scale).r;
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
  float s = s0;
  
  float h = step_size;

  while (s < s1)
  {
    h = min (h, s1 - s);
    
	// 1. grayscale
    //src = vec4(GetFromVolume(s + h));
    //src.a *= 0.5f;
	// 2. using a transfer function (rgba)
    src = GetFromTransferFunction(s + h);
	
	//Front to back blending
	// dst.rgb = dst.rgb + (1 - dst.a) * src.a * src.rgb
	// dst.a   = dst.a   + (1 - dst.a) * src.a		
	src.rgb *= src.a;
	dst = dst + (1.0f - dst.a) * src;
    
    //break from the loop when alpha gets high enough
    if(dst.a >= .95f)
      break; 

    s = s + h;
  }
  FragColor = 
	//vec4(EntryPoint, 1)
	dst
	;
}