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

uniform int rendermethod;
  
int MAXSUBSAMPLES = 2;
float initialDistance = 0.01;
float error_bound =  0.01;
float def_multiplier = 1;
float multiplier = 1;

bool use_transfer_function = true;

vec4 RiemannSum (vec3 frontpos, vec3 raydir, float dmax, float dist)
{
  vec4 color = vec4(0);

  vec4 volumesample;
  vec3 pos = frontpos;
  float value = 0;
  
  for(float ft = 0.0 ; ft < dmax ; ft += dist)
  {
    value = texture(volumeTexture, pos).r;
             
    if(use_transfer_function)
      volumesample = texture(transferFunctionTexture, value);
    else
      volumesample = vec4(vec3(value), value*0.5f);
      
    volumesample.rgb *= volumesample.a;
    color = (1.0f - color.a)*volumesample + color;
    
    pos += raydir*dist;
  }

  return color;
}

vec4 RiemannSumWithSimpsonError (vec3 frontpos, vec3 raydir, float dmax, float dist)
{
  vec4 color = vec4(0);

  vec4 volumesample;
  vec3 a = frontpos;
  vec3 b = a + 2.0 * dist * raydir;
  vec3 c = (a + b) / 2.0;
  float fa, fd, fc, fe, fb, h, S, S2;
  vec3 d, e;
  for(float ft = dist; ft < dmax; ft += 2.0*dist)
  {
    fa = texture(volumeTexture, a).r;
    fc = texture(volumeTexture, c).r;
    fb = texture(volumeTexture, b).r;

    volumesample = texture(transferFunctionTexture, fc);
    volumesample.rgb *= volumesample.a;
    color = (1.0f - color.a)*volumesample + color;      

    volumesample = texture(transferFunctionTexture, fb);
    volumesample.rgb *= volumesample.a;
    color = (1.0f - color.a)*volumesample + color;    
    
    d = (a + c) / 2.0;
    e = (c + b) / 2.0;
    fd = texture(volumeTexture, d).r;
    fe = texture(volumeTexture, e).r;
    h = distance(a,b);
    S = (h/6)*(fa + 4*fc + fb);
    S2 = (h/12)*(fa + 4*fd + fc) + (h/12)*(fc + 4*fe + fb);   

    if(abs(S2 - S) > error_bound)
      dist = dist / 2.0;

    a = b;
    b += 2.0 * dist * raydir;
    c = (a + b) / 2.0;
  }
  
  return color;
}

vec4 SOPIVR (vec3 frontpos, vec3 raydir, float dmax, float dist)
{
  vec4 color = vec4(0);

  float d = dist;
  vec3 a = frontpos;
  vec3 b = a + (2.0 * d) * raydir;
  vec3 c = (a + b) / 2.0;
  float fa, fc, fb;
  float A, B, C, P_t;
  vec4 slabsubsample;
    
  fb = texture(volumeTexture, a).r;
  for(float ft = d; ft < dmax; ft += 2.0*d)
  {
    fa = fb;
    fc = texture(volumeTexture, c).r;
    fb = texture(volumeTexture, b).r;

    //Calculating the quadratic function of next SLAB
    A = 2.0 * fa - 4.0 * fc + 2.0 * fb;
    B = - 3.0 * fa + 4.0 * fc - fb;
    C = fa;
  
    vec4 slab = texture(transferFunctionTexture, A + B + C);
    slab.rgb *= slab.a;

    float substep = 1.0/(MAXSUBSAMPLES);
    float t = 1.0 - substep;
    for (int i = 0; i < MAXSUBSAMPLES; i++, t = t + substep)
    {
      //Back to Front Compositing using Horner's rule
      P_t = (A*t + B) * t + C;
      slabsubsample = texture(transferFunctionTexture, P_t);

      //slabsubsample.a *= multiplier;
      //Back to front composition: Levoy
      slabsubsample.rgb *= slabsubsample.a;
      slab.rgb = slabsubsample.rgb + (1.0 - slabsubsample.a) * slab.rgb;
      slab.a = slabsubsample.a + (1.0 - slabsubsample.a) * slab.a;
    }
    //Front to back slab compositing
	  color = (1.0 - color.a)*slab + color;

    a = b;
    b += 2.0 * d * raydir;
    c = (a + b) / 2.0;     
  }

  return color;
}

vec4 SOPIVRWithSimpsonError (vec3 frontpos, vec3 raydir, float dmax, float dist)
{
  vec4 color = vec4(0);

  vec3 a = frontpos;
  vec3 b = a + 2.0 * dist * raydir;
  vec3 c = (a + b) / 2.0;
  float fa, fd, fc, fe, fb, h, S, S2;
  vec3 d, e;
  float A, B, C, P_t;
  
  float opac_mult = def_multiplier / dmax;
  
  fb = texture(volumeTexture, a).r;
  for(float ft = dist; ft < dmax; ft += 2.0*dist)
  {
    fa = fb;
    fc = texture(volumeTexture, c).r;
    fb = texture(volumeTexture, b).r;

    //Calculating the next SLAB
    A = 2.0 * fa - 4.0 * fc + 2.0 * fb;
    B = - 3.0 * fa + 4.0 * fc - fb;
    C = fa;

    vec4 slab = texture(transferFunctionTexture, A + B + C);
    slab.rgb *= slab.a;
    vec4 slabsubsample = vec4(0);
    float substep = 1.0/(MAXSUBSAMPLES);
    float t = 1 - substep;
    for (int i = 0; i < MAXSUBSAMPLES; i++, t = t - substep)
    {
      //Back to Front Compositing using Horner's rule
      P_t = (A*t + B) * t + C;
      slabsubsample = texture(transferFunctionTexture, P_t);

      slabsubsample.a *= opac_mult / MAXSUBSAMPLES;
      //Back to front composition: Levoy
      slabsubsample.rgb *= slabsubsample.a;
      slab.rgb = slabsubsample.rgb + (1.0 - slabsubsample.a) * slab.rgb;
      slab.a = slabsubsample.a + (1.0 - slabsubsample.a) * slab.a;
    }
    //Front to back slab compositing
	  color = (1.0 - color.a)*slab + color;

    float error = 0.0;
    d = (a + c) / 2.0;
    e = (c + b) / 2.0;
    fd = texture(volumeTexture, d).r;
    fe = texture(volumeTexture, e).r;
    h = distance(a,b);

    S = (h / 6.0)*(fa + 4.0 * fc + fb);
    S2 = (h / 12.0)*(fa + 4.0 * fd + fc) + (h / 12.0)*(fc + 4.0 * fe + fb);
    error = abs (S2 - S);

    
    if (error > error_bound)
      if (MAXSUBSAMPLES < 64)
        MAXSUBSAMPLES = 2 * MAXSUBSAMPLES;
    else if (error == 0.0)
      if (MAXSUBSAMPLES > 1)
        MAXSUBSAMPLES = MAXSUBSAMPLES / 2;
        
    a = b;
    b += 2.0 * dist * raydir;
    c = (a + b) / 2.0;
  }
  
  return color;
}

vec4 SOPIVRWithSimpsonSecError (vec3 frontpos, vec3 raydir, float dmax, float dist, int typeerroradjust)
{
  vec4 color = vec4(0);

  vec3 a = frontpos;
  vec3 b = a + 2.0 * dist * raydir;
  vec3 c = (a + b) / 2.0;
  float fa, fc, fb, fd, fe;
  float A, B, C, P_t, h, S, S2;
  vec4 slabsubsample;
 
  fb = texture(volumeTexture, a).r;
  for(float ft = dist; ft < dmax; ft += 2.0*dist)
  {
    float error = 0.0f;

    fa = fb;
    fc = texture(volumeTexture, c).r;
    fb = texture(volumeTexture, b).r;

    //Calculate error
    fd = texture(volumeTexture, (a+c)/2.0f).r;
    fe = texture(volumeTexture, (c+b)/2.0f).r;
    h = 2.0f*dist;

    S = (h / 6.0)*(fa + 4.0 * fc + fb);
    S2 = (h / 12.0)*(fa + 4.0 * fd + fc) + (h / 12.0)*(fc + 4.0 * fe + fb);
    error = abs (S2 - S);
       
    //compute error
    if (typeerroradjust == 0)
    {
      if (error > error_bound)
        MAXSUBSAMPLES = 2 * MAXSUBSAMPLES;
      else if (error == 0.0)
        if (MAXSUBSAMPLES > 1)
          MAXSUBSAMPLES = MAXSUBSAMPLES / 2;
    }
    else
    {
      if (error > error_bound)
        if (dist > 0.1)
          dist = dist / 2.0f;
      else if (error == 0.0)
        dist = dist * 2.0f;
    }
    //*compute error
    //*Calculate error

    //Calculating the next SLAB
    A = 2.0 * fa - 4.0 * fc + 2.0 * fb;
    B = - 3.0 * fa + 4.0 * fc - fb;
    C = fa;

    vec4 slab = texture(transferFunctionTexture, A + B + C);
    slab.rgb *= slab.a;

    float substep = 1.0/(MAXSUBSAMPLES);
    float t = 1.0 - substep;
    for (int i = 0; i < MAXSUBSAMPLES; i++, t = t + substep)
    {
      //Back to Front Compositing using Horner's rule
      P_t = (A*t + B) * t + C;
      slabsubsample = texture(transferFunctionTexture, P_t);

      //slabsubsample.a *= multiplier;
      //Back to front composition: Levoy
      slabsubsample.rgb *= slabsubsample.a;
      slab.rgb = slabsubsample.rgb + (1.0 - slabsubsample.a) * slab.rgb;
      slab.a = slabsubsample.a + (1.0 - slabsubsample.a) * slab.a;
    }
    //Front to back slab compositing
    color = (1.0 - color.a)*slab + color;
        
    a = b;
    b += 2.0 * dist * raydir;
    c = (a + b) / 2.0;
  }
  
  return color;
}

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
  multiplier = 1 / dmax;
  if (dist > dmax)
    dist = dmax;

  vec4 finalcolor = vec4(0);
  vec4 volumesample = vec4(0);

  //1
  if (rendermethod == 0) finalcolor = SOPIVR (front, raydir, dmax, dist);
  //2
  else if (rendermethod == 1) finalcolor = SOPIVRWithSimpsonSecError (front, raydir, dmax, dist, 1);
  //3
  else if (rendermethod == 2) finalcolor = SOPIVRWithSimpsonError (front, raydir, dmax, dist);
  //4
  else if (rendermethod == 3) finalcolor = RiemannSum (front, raydir, dmax, dist/MAXSUBSAMPLES);
  //5
  else if (rendermethod == 4) finalcolor = RiemannSumWithSimpsonError (front, raydir, dmax, dist/MAXSUBSAMPLES);
  //6
  else if (rendermethod == 5) finalcolor = vec4(vec3(abs(RiemannSum (front, raydir, dmax, dist/MAXSUBSAMPLES) - SOPIVR(front, raydir, dmax, dist)).rgb), 1);
  //7
  //else if (rendermethod == 6) finalcolor = vec4(vec3(abs(SOPIVRWithSimpsonSecError (front, raydir, dmax, dist) - SOPIVR(front, raydir, dmax, dist)).rgb), 1);
  //else if (rendermethod == 7)
  //else if (rendermethod == 8) finalcolor = vec4(0);
  //else if (rendermethod == 9) finalcolor = vec4(0);
  
  volumeOut = finalcolor;
}

/*

vec4 SOPIVRWithSimpsonSubError (vec3 frontpos, vec3 raydir, float dmax, float dist)
{
  vec4 color = vec4(0);

  vec3 a = frontpos;
  vec3 b = a + 2.0 * dist * raydir;
  vec3 c = (a + b) / 2.0;
  float fa, fc, fb;
  float A, B, C, P_t;
  vec4 slabsubsample;
 
  float ea, eb, ec, ed, ee;
  float efa, efd, efc, efe, efb, h, S, S2;
  
  fb = texture(volumeTexture, a).r;
  for(float ft = dist; ft < dmax; ft += 2.0*dist)
  {
    fa = fb;
    fc = texture(volumeTexture, c).r;
    fb = texture(volumeTexture, b).r;

    //Calculating the next SLAB
    A = 2.0 * fa - 4.0 * fc + 2.0 * fb;
    B = - 3.0 * fa + 4.0 * fc - fb;
    C = fa;

    vec4 slab = texture(transferFunctionTexture, A + B + C);
    slab.rgb *= slab.a;

    float substep = 1.0/(MAXSUBSAMPLES);
    float t = 1.0 - substep;
    float error = 0.0f;
    for (int i = 0; i < MAXSUBSAMPLES; i++, t = t + substep)
    {
      //Back to Front Compositing using Horner's rule
      P_t = (A*t + B) * t + C;
      slabsubsample = texture(transferFunctionTexture, P_t);

      //Back to front composition: Levoy
      slabsubsample.rgb *= slabsubsample.a;
      slab.rgb = slabsubsample.rgb + (1.0 - slabsubsample.a) * slab.rgb;
      slab.a = slabsubsample.a + (1.0 - slabsubsample.a) * slab.a;

      if (i + 1 < MAXSUBSAMPLES)
      {
        ea = t - substep;
        eb = t + substep;
        ec = t;
        efa = (A*ea + B) * ea + C;
        efc = (A*ec + B) * ec + C;
        efb = (A*eb + B) * eb + C;

        ed = (ea + ec) / 2.0;
        ee = (ec + eb) / 2.0;
        efd = (A*ed + B) * ed + C;
        efe = (A*ee + B) * ee + C;
        h = substep * 2.0f;

        S = (h / 6.0)*(efa + 4.0 * efc + efb);
        S2 = (h / 12.0)*(efa + 4.0 * efd + efc) + (h / 12.0)*(efc + 4.0 * efe + efb);
        error = error + abs (S2 - S);
      }
    }

    //Front to back slab compositing
    color = (1.0 - color.a)*slab + color;

    if (error > error_bound)
      MAXSUBSAMPLES = 2 * MAXSUBSAMPLES;
    else if (error == 0.0)
      if (MAXSUBSAMPLES > 1)
        MAXSUBSAMPLES = MAXSUBSAMPLES / 2;
        
    a = b;
    b += 2.0 * dist * raydir;
    c = (a + b) / 2.0;
  }
  
  return color;
}

*/