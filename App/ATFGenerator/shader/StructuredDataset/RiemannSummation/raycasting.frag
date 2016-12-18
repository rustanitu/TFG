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
uniform usamplerBuffer ActiveTex;
uniform sampler1D TransferFunc;  

uniform int VolWidth;
uniform int VolHeight;
uniform int VolDepth;

uniform int VisibleSet;

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
	vec3 voxel = (real_minpos + p_d * real_normalized_step) * tex_scale;
	float voxel_value = texture(VolumeTex, voxel).r;
	uint active_cell = texelFetch(ActiveTex, int(voxel.x + (VolWidth * voxel.y) + (voxel.z * VolWidth * VolHeight))).r;
	if (active_cell == 1)
		return texture(TransferFunc, voxel_value);
	return vec4(0);
}

void main(void)
{
	vec4 colorAcum = vec4(0.0);
	float transparencyAcum = 1.0;

	vec3 expoint = texture(ExitPoints, gl_FragCoord.st / vec2(ScreenSizeW, ScreenSizeH)).xyz;
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
	////////////////////////////////////////////
	while (s < s1)
	{
		h = min (h, s1 - s);
		
		vec4 src = GetFromTransferFunction(s + h);
		
		float d = h;
		colorAcum.x += src.x*src.w*d * transparencyAcum;
		colorAcum.y += src.y*src.w*d * transparencyAcum;
		colorAcum.z += src.z*src.w*d * transparencyAcum;
		colorAcum.w += src.w*d       * transparencyAcum;
	
		transparencyAcum *= (1.0 - src.w*d);
	
		s = s + h;
	}
	//colorAcum.w = 1 - transparencyAcum;
	////////////////////////////////////////////////
	/*float exp_opacity = 1.0;
	float opacity = 0.0;
	float a_internal, h_12;

	vec4 tf_a, tf_c, tf_d, tf_e, F_a, F_c, F_b;
	vec4 tf_b = GetFromTransferFunction (s0);
	
	while (s < s1)
	{
		h = min (h, s1 - s);
	
		h_12 = h / 12.0;
	
		tf_a = tf_b;
		tf_c = GetFromTransferFunction (s + h * 0.5);  
		tf_b = GetFromTransferFunction (s + h);        
		tf_d = GetFromTransferFunction (s + h * 0.125);
		tf_e = GetFromTransferFunction (s + h * 0.375);
	
		a_internal = tf_a.w * exp_opacity;
		F_a = vec4 (tf_a.x*a_internal, tf_a.y*a_internal, tf_a.z*a_internal, a_internal);
	
		opacity += h_12*(tf_a.w + 4.0 * tf_d.w + tf_c.w);
		a_internal = tf_c.w * exp (-opacity);
		F_c = vec4 (tf_c.x*a_internal, tf_c.y*a_internal, tf_c.z*a_internal, a_internal);
	
		opacity += h_12*(tf_c.w + 4.0 * tf_e.w + tf_b.w);
		exp_opacity = exp (-opacity);
		a_internal = tf_b.w * exp_opacity;
		F_b = vec4 (tf_b.x*a_internal, tf_b.y*a_internal, tf_b.z*a_internal, a_internal);
	
		colorAcum += (h_12 * 2.0) * (F_a + 4.0 * F_c + F_b);
	
		s = s + h;
	}*/
	//////////////////////////////////////
	FragColor = colorAcum;

	//FragColor = vec4(EntryPoint, 1);
	//FragColor = vec4(expoint, 1);
}