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

in vec4 ex_Position;
in vec2 ex_TextCoord; //between 0 and 1
layout (location = 0) out vec4 colorOut;
layout (location = 1) out vec4 volumeOut;

uniform sampler2D backTexture;
uniform sampler2D frontTexture;
uniform sampler1D transferFunctionTexture;
uniform sampler3D volumeTexture;
uniform sampler3D gradientTexture;
uniform vec3 proportion;

uniform int use_gradient;
uniform int samples;

void main(void)
{
  volumeOut = vec4(0);
  vec4 cfront = texture2D(frontTexture, ex_TextCoord);
  vec4 cback = texture2D(backTexture, ex_TextCoord);
  if(cback == cfront)
  {
    discard;
  }
  colorOut = vec4((cback - cfront).rgb, cfront.a);

  vec3 front = cfront.rgb;
  vec3 back = cback.rgb;

  vec3 dir = normalize(back - front);
  vec4 pos = vec4(front, 0);
  
  vec4 dst = vec4(0);
  vec4 src = vec4(0);

  vec4 value;
  vec3 gvalue;

  float StepSize = (distance(back,front)/samples);
  vec3 Step = dir*StepSize;

  //this should really be an extern variable
  vec3 L = vec3(0.0, 1.0, 1.0);
  float BaseSampleDist = .7f;
  float ActualSampleDist = .7f;

  for(int i = 0; i < samples; i++)
  {
    pos.w = 0;
    value = texture(volumeTexture, vec3(pos.x, pos.y, pos.z));     
    
    src = texture(transferFunctionTexture, value.r);

    if(use_gradient == 1)
    {
      gvalue = texture(gradientTexture, pos.xyz).rgb;
   
      //Oppacity correction: As sampling distance decreases we get more samples.
	    //Therefore the alpha values set for a sampling distance of .5f will be too
	    //high for a sampling distance of .25f (or conversely, too low for a sampling
	    //distance of 1.0f). So we have to adjust the alpha accordingly.
	    src.a = 1 - pow((1 - src.a), ActualSampleDist / BaseSampleDist);
		
      float s = dot(gvalue, L);			  
					
		  //diffuse shading + fake ambient lighting
		  src.rgb = s * src.rgb + .1f * src.rgb;
		}

		//Front to back blending
		// dst.rgb = dst.rgb + (1 - dst.a) * src.a * src.rgb
		// dst.a   = dst.a   + (1 - dst.a) * src.a		
		src.rgb *= src.a;
		dst = (1.0f - dst.a)*src + dst;
		
		//break from the loop when alpha gets high enough
		if(dst.a >= .95f)
			break;	
		
		//advance the current position
		pos.xyz += Step;
     
    if(pos.x > 1.0f &&  pos.y > 1.0f && pos.z > 1.0f)
      break;
  }
  volumeOut = dst;
}