/*
         +1
     ----------
     |        |
  -1 |        | +1
     |        |
     ----------
         -1
*/

#version 330

precision highp float;

in vec4 ex_Position;
in vec2 ex_TextCoord; //between 0 and 1
layout (location = 0) out vec4 colorOut;
layout (location = 1) out vec4 volumeOut;

uniform sampler2D backTexture;
uniform sampler2D frontTexture;

uniform sampler1D transferFunctionTexture;
uniform sampler3D volumeTexture;
uniform sampler3D gradientTexture;

uniform int usegradient;
uniform int samples;

uniform float cube_width;
uniform float cube_height;
uniform float cube_depth;

////// Extern Variables //////
vec4 dst = vec4(0);
vec4 src = vec4(0);

vec3 L = vec3(0.0, 1.0, 1.0);
float BaseSampleDist = .5f;
float ActualSampleDist = .5f;
////// Extern Variables //////

void main(void)
{
  volumeOut = vec4(0);
  vec4 cfront = texture(frontTexture, ex_TextCoord);
  vec4 cback = texture(backTexture, ex_TextCoord);
  if(cback == cfront)
    discard;

  // rays texture
  colorOut = vec4((cback - cfront).rgb, cfront.a);

  vec3 front = cfront.rgb;
  vec3 back = cback.rgb;

  vec3 dir = normalize(back - front);
  vec3 pos = vec3(front.x * (1.0f/cube_width ), 
                  front.y * (1.0f/cube_height), 
                  front.z * (1.0f/cube_depth));

  float StepSize = (distance(back, front)/samples);
  vec3 Step = dir * StepSize;
  Step = vec3(Step.x * (1.0f/cube_width),
              Step.y * (1.0f/cube_height),
              Step.z * (1.0f/cube_depth));

  for(int i = 0; i < samples; i++)
  {
    src = vec4(0);
    float value = texture(volumeTexture, pos).r;  
    src = texture(transferFunctionTexture, value);

    //Oppacity correction
	  src.a = 1 - pow((1 - src.a), ActualSampleDist / BaseSampleDist);
    
    if(usegradient == 1)
    {
      vec3 gvalue = texture(gradientTexture, pos).rgb;
      float s = dot(gvalue, L);			  
		  //diffuse shading + fake ambient lighting
		  src.rgb = s * src.rgb + .4f * src.rgb;
		}

		//Front to back blending
		// dst.rgb = dst.rgb + (1 - dst.a) * src.a * src.rgb
		// dst.a   = dst.a   + (1 - dst.a) * src.a		
		src.rgb *= src.a;
		dst = (1.0f - dst.a)*src + dst;
		if(dst.a >= .95f)
			break;	
		
		pos += Step;
    if(pos.x > 1.0f &&  pos.y > 1.0f && pos.z > 1.0f &&
       pos.x < 0.0f &&  pos.y < 0.0f && pos.z < 0.0f)
      break;
  }
  volumeOut = dst;
  //volumeOut = texture(backTexture, ex_TextCoord);
  //volumeOut = texture(frontTexture, ex_TextCoord);
}