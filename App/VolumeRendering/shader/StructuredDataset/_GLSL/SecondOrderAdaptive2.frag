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

unsigned int usegradient;
unsigned int samples;

float d = 0.001;

bool use_transfer_function = true;

void main(void)
{
  vec4 cfront = texture2D(frontTexture, ex_TextCoord);
  vec4 cback = texture2D(backTexture, ex_TextCoord);
  if(cback == cfront)
    discard;

  colorOut = vec4((cback - cfront).rgb, cfront.a);
  vec3 front = cfront.rgb;
  vec3 back = cback.rgb;

  vec3 raydir = normalize(back - front);
  
  float dmax = distance(front, back);  
  float multiplier = 1 / dmax;
  if (d > dmax)
    d = dmax;

  vec4 finalcolor = vec4(0);
  vec4 volumesample = vec4(0);

  vec3 a = front;
  vec3 b = a + (2.0 * d) * raydir;
  vec3 c = (a + b) / 2.0;
  float fa, fc, fb;
  float A, B, C, P_t;
  
  for(float ft = d; ft < dmax; ft += 2.0*d)
  {
    fa = texture(volumeTexture, a).r;
    fc = texture(volumeTexture, c).r;
    fb = texture(volumeTexture, b).r;

    //Calculating the quadratic function of next SLAB
    A = 2.0 * fa - 4.0 * fc + 2.0 * fb;
    B = - 3.0 * fa + 4.0 * fc - fb;
    C = fa;
    
    vec4 slab = vec4(0) 
    + texture(transferFunctionTexture, A + B + C)
    ;
    slab.rgb *= slab.a;
    vec4 slabsubsample = vec4(0);

    float substep = 1.0/(samples);
    float t = 1.0 - substep;
    for (int i = 0; i < samples; i++, t = t + substep)
    {
      //Back to Front Compositing using Horner's rule
      P_t = (A*t + B) * t + C;
      slabsubsample = texture(transferFunctionTexture, P_t);

      slabsubsample.a *= multiplier;
      //Back to front composition: Levoy
      slabsubsample.rgb *= slabsubsample.a;
      slab.rgb = slabsubsample.rgb + (1.0 - slabsubsample.a) * slab.rgb;
      slab.a = slabsubsample.a + (1.0 - slabsubsample.a) * slab.a;
      //slab.rgb = (1 - slabsubsample.a)*slab.rgb + slabsubsample.rgb*slabsubsample.a;
    }
    //Front to back slab compositing
    //slab.rgb *= slab.a; // esta multiplicação já foi feita antes do for de t [0,1]
		finalcolor = (1.0 - finalcolor.a)*slab + finalcolor;
    //finalcolor.rgb = finalcolor.rgb + (1-finalcolor.a)*slab.rgb;
    //finalcolor.a = finalcolor.a + (1-finalcolor.a)*slab.a;


    a = b;
    b += 2.0 * d * raydir;
    c = (a + b) / 2.0;
  }
  volumeOut = finalcolor;
}
