#version 330

in vec4 ex_Position;
in vec2 ex_TextCoord;

//between 0 and 1
layout (location = 0) out vec4 colorOut;
layout (location = 1) out vec4 volumeOut;

uniform sampler2D backTexture;
uniform sampler2D frontTexture;
uniform sampler1D transferFunctionTexture;
uniform sampler3D volumeTexture;
uniform sampler3D gradientTexture;

uniform int rendermethod;
  
float opacityCorrectionDistance = 0.005;
float initialDistance = 0.05;
float error_bound =  0.00001;
const int errorzeromaxstacks = 20;
bool showrefinments = false;
int sub_samples = 8;

void main(void)
{
  volumeOut = vec4(0);
  vec4 cfront = texture2D(frontTexture, ex_TextCoord);
  vec4 cback = texture2D(backTexture, ex_TextCoord);
  if(cback == cfront)
    discard;
  
  colorOut = vec4((cback - cfront).rgb, cfront.a);
  vec3 front = cfront.rgb;
  vec3 back = cback.rgb;
  
  float dmax = distance(front, back);  
  vec3 pos = front;
  
  vec3 raydir = normalize(back - front);
  float dist = initialDistance;
  if (dist > dmax)
    dist = dmax;

  vec4 finalcolor = vec4(0);
  vec4 volumesample = vec4(0);

  vec4 color = vec4(0);

  vec3 a = front;
  vec3 b = a + 2.0 * dist * raydir;
  vec3 c = (a + b) / 2.0;
  float fa, fd, fc, fe, fb, h, S1, S2;
  vec3 d, e;
  float A, B, C, P_t;
  float error;
    int errorzerostacks = 0;
  float substep = 1.0/sub_samples;
        
  fb = texture(volumeTexture, a).r;

  #pragma optionNV(unroll none)
  for(float ft = dist; ft < dmax; ft += 2.0*dist)
  {
    fa = fb;
    fc = texture(volumeTexture, c).r;
    fb = texture(volumeTexture, b).r;

    //Interpolate next slab
    A = 2.0 * fa - 4.0 * fc + 2.0 * fb;
    B = - 3.0 * fa + 4.0 * fc - fb;
    C = fa;

    vec4 slab = texture(transferFunctionTexture, A + B + C);
    vec4 slabsubsample = vec4(0);
    float t = 1 - substep;
    
    //"opacity correction in raycasting"
    slab.a = 1 - pow(1 - slab.a, dist/opacityCorrectionDistance);
    slab.a = 1 - pow(1 - slab.a, substep);
    
    slab.rgb *= slab.a;
    for (int i = 0; i < sub_samples; i++, t = t - substep)
    {
      //Back to Front Compositing using Horner's rule
      P_t = (A*t + B) * t + C;
      slabsubsample = texture(transferFunctionTexture, P_t);

      //"opacity correction in raycasting"
      slabsubsample.a = 1 - pow(1 - slabsubsample.a, dist/opacityCorrectionDistance);
      slabsubsample.a = 1 - pow(1 - slabsubsample.a, substep);

      //Back to front composition: Levoy
      slabsubsample.rgb *= slabsubsample.a;
      slab.rgb = slabsubsample.rgb + (1.0 - slabsubsample.a) * slab.rgb;
      slab.a = slabsubsample.a + (1.0 - slabsubsample.a) * slab.a;
    }
    //Front to back slab compositing
	  color = (1.0 - color.a)*slab + color;

    //calculate error using adaptive simpson method
    error = 0.0;
    d = (a + c) / 2.0;
    e = (c + b) / 2.0;
    fd = texture(volumeTexture, d).r;
    fe = texture(volumeTexture, e).r;
    h = distance(a,b);
    S1 = (h / 6.0)*(fa + 4.0 * fc + fb);
    S2 = (h / 12.0)*(fa + 4.0 * fd + fc) + (h / 12.0)*(fc + 4.0 * fe + fb);
    error = abs(S2 - S1);

    if (error >= 15.0*error_bound/sub_samples)
    {
      errorzerostacks = 0;
      if (rendermethod == 0)
      {
        if (sub_samples < 64)
        {
          sub_samples = 2 * sub_samples;
          substep = 1.0/sub_samples;
          if (showrefinments)
          {
            color = vec4(1,0,0,1);
            break;
          }
        }
      }
      else
      {
        dist = dist / 2.0;
        if (showrefinments)
        {
          color = vec4(1,0,0,1);
          break;
        }
      }
    }
    else if (error == 0.0)
    {
      errorzerostacks++;
      if (errorzerostacks == errorzeromaxstacks)
      {
        if (rendermethod == 0)
        {
          if (sub_samples > 1)
          {
            sub_samples = sub_samples / 2;
            substep = 1.0/sub_samples;
          }
        }
        else
        {
          dist = dist * 2;
          errorzerostacks = 0;
        }
      }
    }
    else
    {
      errorzerostacks = 0;
    } 

    //next sample
    a = b;
    b += 2.0 * dist * raydir;
    c = (a + b) / 2.0;
  }
  volumeOut = color;
}