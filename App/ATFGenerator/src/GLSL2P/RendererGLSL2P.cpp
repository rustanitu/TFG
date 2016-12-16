#include "RendererGLSL2P.h"

#include "../gbuffer.h"

#include <volrend/ScalarField.h>
#include <volrend/TransferFunction.h>
#include <glutils/GLUtils.h>
#include <math/MUtils.h>
#include <lqc/lqcdefines.h>

#include <volrend/Utils.h>

#include <fstream>

RendererGLSL2P::RendererGLSL2P ()
: m_FrameCount (0),
m_LastTime (0),
m_x_rotation (0.0f),
m_y_rotation (0.0f),
m_glfbo (NULL),
m_iterate (false),
m_glsl_volume (NULL),
m_glsl_transfer_function (NULL),
m_glsl_settex(NULL),
m_glsl_setqtdtex(NULL),
m_shader_firstpass(NULL),
m_shader_secondpass(NULL)
{
	m_fvao = NULL; m_fvbo = NULL; m_fibo = NULL;
	m_svao = NULL; m_svbo = NULL; m_sibo = NULL;

	m_cube_width = 1.0f;
	m_cube_height = 1.0f;
	m_cube_depth = 1.0f;

	m_init_slice_x = 0;
	m_last_slice_x = 0;
	m_init_slice_y = 0;
	m_last_slice_y = 0;
	m_init_slice_z = 0;
	m_last_slice_z = 0;

	USE_DOUBLE_PRECISION = false;
}

RendererGLSL2P::~RendererGLSL2P ()
{
	Destroy ();
}

void RendererGLSL2P::CreateScene (int CurrentWidth, int CurrentHeight, vr::ScalarField* volume, bool resetslices)
{
	m_sdr_width = CurrentWidth;
	m_sdr_height = CurrentHeight;
	m_ModelMatrix = lqc::IDENTITY_MATRIX;
	m_ProjectionMatrix = lqc::IDENTITY_MATRIX;
	m_ViewMatrix = lqc::IDENTITY_MATRIX;
	float zcam = -2.0f;
	lqc::TranslateMatrix (&m_ViewMatrix, 0, 0, zcam);

	m_glfbo = new GBuffer (CurrentWidth, CurrentHeight);

	if (resetslices)
	{
		m_init_slice_x = 0;
		m_last_slice_x = volume->GetWidth ();
		m_init_slice_y = 0;
		m_last_slice_y = volume->GetHeight ();
		m_init_slice_z = 0;
		m_last_slice_z = volume->GetDepth ();
	}
 
	CreateFirstPass ();
	CreateSecondPass ();

	Resize (CurrentWidth, CurrentHeight);

	BindShaderUniforms ();
}

bool RendererGLSL2P::Render (int Width, int Height)
{
	m_ModelMatrix = lqc::IDENTITY_MATRIX;
	lqc::TranslateMatrix (&m_ModelMatrix, -(m_cube_width / 2.0f), -(m_cube_height / 2.0f), -(m_cube_depth / 2.0f));
	lqc::RotateAboutX (&m_ModelMatrix, m_x_rotation * (float)PI / 180.0f);
	lqc::RotateAboutY (&m_ModelMatrix, m_y_rotation * (float)PI / 180.0f);

	m_glfbo->Bind ();

	FirstPass ();
	SecondPass ();
	
	m_glfbo->renderBuffer (Width, Height, 0);

	GBuffer::Unbind ();
	return true;
}

void RendererGLSL2P::Resize (int Width, int Height)
{
	m_sdr_width = Width;
	m_sdr_height = Height;
	glViewport (0, 0, Width, Height);
	m_ProjectionMatrix =
		lqc::CreateProjectionMatrix (
		30,
		(float)Width / (float)Height,
		1.0f,
		100.0f
		);

	if (!m_shader_firstpass)
		return;

	m_shader_firstpass->Bind ();
	m_shader_firstpass->SetUniformMatrix4f ("ProjectionMatrix", m_ProjectionMatrix);
	m_shader_firstpass->BindUniform ("ProjectionMatrix");
	m_shader_firstpass->Unbind ();

	if (!m_shader_secondpass)
		return;

	m_shader_secondpass->Bind ();
	m_shader_secondpass->SetUniformMatrix4f ("ProjectionMatrix", m_ProjectionMatrix);
	m_shader_secondpass->BindUniform ("ProjectionMatrix");
	m_shader_secondpass->SetUniformInt ("ScreenSizeW", m_sdr_width);
	m_shader_secondpass->BindUniform ("ScreenSizeW");
	m_shader_secondpass->SetUniformInt ("ScreenSizeH", m_sdr_height);
	m_shader_secondpass->BindUniform ("ScreenSizeH");
	m_shader_secondpass->Unbind ();

	m_glfbo->resize (Width, Height);
}

void RendererGLSL2P::CreateFirstPass ()
{
	float wx = m_cube_width;
	float hy = m_cube_height;
	float dz = m_cube_depth;

	const RendererGLSL2P::Vertex VERTICES[8] =
	{
		{ { 0.0f, 0.0f,   dz }, { 0.0f, 0.0f,   dz } },
		{ { 0.0f,   hy,   dz }, { 0.0f,   hy,   dz } },
		{ {   wx,   hy,   dz }, {   wx,   hy,   dz } },
		{ {   wx, 0.0f,   dz }, {   wx, 0.0f,   dz } },
		{ { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } },
		{ { 0.0f,   hy, 0.0f }, { 0.0f,   hy, 0.0f } },
		{ {   wx,   hy, 0.0f }, {   wx,   hy, 0.0f } },
		{ {   wx, 0.0f, 0.0f }, {   wx, 0.0f, 0.0f } }
	};

	const GLuint INDICES[36] =
	{
		0, 2, 1, 0, 3, 2,
		4, 3, 0, 4, 7, 3,
		4, 1, 5, 4, 0, 1,
		3, 6, 2, 3, 7, 6,
		1, 6, 5, 1, 2, 6,
		7, 5, 6, 7, 4, 5
	};

	m_shader_firstpass = new gl::GLShader ("shader/StructuredDataset/backface.vert",
		"shader/StructuredDataset/backface.frag");
	m_shader_firstpass->SetUniformMatrix4f ("ModelMatrix", lqc::IDENTITY_MATRIX);
	m_shader_firstpass->SetUniformMatrix4f ("ViewMatrix", lqc::IDENTITY_MATRIX);
	m_shader_firstpass->SetUniformMatrix4f ("ProjectionMatrix", lqc::IDENTITY_MATRIX);
	gl::ExitOnGLError ("ERROR: Could not get shader uniform locations");

	gl::GLShader::Unbind ();

	m_fvao = new gl::GLArrayObject (2);
	m_fvao->Bind ();

	m_fvbo = new gl::GLBufferObject (gl::GLBufferObject::TYPES::VERTEXBUFFEROBJECT);
	m_fibo = new gl::GLBufferObject (gl::GLBufferObject::TYPES::INDEXBUFFEROBJECT);

	//bind the VBO to the VAO
	m_fvbo->SetBufferData (sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);

	m_fvao->SetVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)0);
	m_fvao->SetVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)sizeof(VERTICES[0].Position));

	//bind the IBO to the VAO
	m_fibo->SetBufferData (sizeof(INDICES), INDICES, GL_STATIC_DRAW);

	gl::GLArrayObject::Unbind ();
}

void RendererGLSL2P::DestroyCube ()
{
	if (m_fvbo)
		delete m_fvbo;
	m_fvbo = NULL;

	if (m_fibo)
		delete m_fibo;
	m_fibo = NULL;

	if (m_fvao)
		delete m_fvao;
	m_fvao = NULL;

	if (m_shader_firstpass)
		delete m_shader_firstpass;
	m_shader_firstpass = NULL;
}

void RendererGLSL2P::DestroyQuad ()
{
	if (m_svbo)
		delete m_svbo;
	m_svbo = NULL;

	if (m_sibo)
		delete m_sibo;
	m_sibo = NULL;

	if (m_svao)
		delete m_svao;
	m_svao = NULL;

	if (m_shader_secondpass)
		delete m_shader_secondpass;
	m_shader_secondpass = NULL;
}

void RendererGLSL2P::Destroy ()
{
	DestroyCube ();
	DestroyQuad ();
	gl::ExitOnGLError ("ERROR: Could not destroy the shaders");

	////////////////////////////////
	if (m_glsl_volume)
		delete m_glsl_volume;
	m_glsl_volume = NULL;

	if (m_glsl_transfer_function)
		delete m_glsl_transfer_function;
	m_glsl_transfer_function = NULL;
	////////////////////////////////

	//////////////////
	if (m_glfbo)
		delete m_glfbo;
	m_glfbo = NULL;
	//////////////////
}

void RendererGLSL2P::BindShaderUniforms ()
{
	m_shader_firstpass->Bind ();
	m_shader_firstpass->BindUniforms ();
	m_shader_firstpass->Unbind ();

	m_shader_secondpass->Bind ();
	m_shader_secondpass->BindUniforms ();
	m_shader_secondpass->Unbind ();
}

void RendererGLSL2P::ResetGeometry ()
{
	DestroyCube ();
	DestroyQuad ();

	CreateFirstPass ();
	CreateSecondPass ();

	BindShaderUniforms ();
}

void RendererGLSL2P::DrawCube (void)
{
	m_shader_firstpass->Bind ();

	m_fvao->DrawElements (GL_TRIANGLES, 36, GL_UNSIGNED_INT);

	gl::GLArrayObject::Unbind ();
	gl::GLShader::Unbind ();
}

//Draw a cube’s front faces into surface A and back faces into surface B. This determines ray intervals.
// Attach two textures to the current FBO to render to both surfaces simultaneously.
//Use a fragment shader that writes out normalized object-space coordinates to the RGB channels.
void RendererGLSL2P::FirstPass ()
{
	m_shader_firstpass->Bind ();
	m_shader_firstpass->SetUniformMatrix4f ("ModelMatrix", m_ModelMatrix);
	m_shader_firstpass->SetUniformMatrix4f ("ViewMatrix", m_ViewMatrix);

	m_shader_firstpass->BindUniform ("ModelMatrix");
	m_shader_firstpass->BindUniform ("ViewMatrix");

	gl::GLShader::Unbind ();

	{
		glCullFace (GL_FRONT);
		GLuint attachments[1] = { GL_COLOR_ATTACHMENT1 };
		glDrawBuffers (1, attachments);
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		DrawCube ();
	}

	glCullFace (GL_BACK);
}

//Draw a full-screen quad to perform the raycast.
// Bind three textures: the two interval surfaces, and the 3D texture you’re raycasting against.
// Sample the two interval surfaces to obtain ray start and stop points. If they’re equal, issue a discard
void RendererGLSL2P::SecondPass ()
{
	m_shader_secondpass->Bind ();
	m_shader_secondpass->SetUniformMatrix4f ("ModelMatrix", m_ModelMatrix);
	m_shader_secondpass->SetUniformMatrix4f ("ViewMatrix", m_ViewMatrix);
	m_shader_secondpass->BindUniform ("ModelMatrix");
	m_shader_secondpass->BindUniform ("ViewMatrix");
	m_shader_secondpass->BindUniform ("ExitPoints");
	gl::GLShader::Unbind ();

	glCullFace (GL_BACK);
	GLuint attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers (1, attachments);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_shader_secondpass->Bind ();

	m_svao->DrawElements (GL_TRIANGLES, 36, GL_UNSIGNED_INT);

	gl::GLArrayObject::Unbind ();
	gl::GLShader::Unbind ();
}

void RendererGLSL2P::CreateSecondPass ()
{
	float wx = m_cube_width;
	float hy = m_cube_height;
	float dz = m_cube_depth;

	const RendererGLSL2P::Vertex VERTICES[8] =
	{
		{ { 0.0f, 0.0f, dz   }, { 0.0f, 0.0f, dz   } },
		{ { 0.0f, hy  , dz   }, { 0.0f, hy  , dz   } },
		{ { wx  , hy  , dz   }, { wx  , hy  , dz   } },
		{ { wx  , 0.0f, dz   }, { wx  , 0.0f, dz   } },
		{ { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } },
		{ { 0.0f, hy  , 0.0f }, { 0.0f, hy  , 0.0f } },
		{ { wx  , hy  , 0.0f }, { wx  , hy  , 0.0f } },
		{ { wx  , 0.0f, 0.0f }, { wx  , 0.0f, 0.0f } }
	};

	const GLuint INDICES[36] =
	{
		0, 2, 1, 0, 3, 2,
		4, 3, 0, 4, 7, 3,
		4, 1, 5, 4, 0, 1,
		3, 6, 2, 3, 7, 6,
		1, 6, 5, 1, 2, 6,
		7, 5, 6, 7, 4, 5
	};

	m_shader_secondpass = new gl::GLShader ("shader/StructuredDataset/raycasting.vert",
		"shader/StructuredDataset/RiemannSummation/raycasting.frag"
		//"shader/StructuredDataset/Composition/fronttoback.frag"
		//"shader/StructuredDataset/Composition/backtofront.frag"
	);

	m_shader_secondpass->SetUniformMatrix4f ("ModelMatrix", lqc::IDENTITY_MATRIX);
	m_shader_secondpass->SetUniformMatrix4f ("ViewMatrix", lqc::IDENTITY_MATRIX);
	m_shader_secondpass->SetUniformMatrix4f ("ProjectionMatrix", lqc::IDENTITY_MATRIX);
	
	m_shader_secondpass->SetUniformTexture2D ("ExitPoints", m_glfbo->m_backtexture, 0);
	if (m_glsl_volume) m_shader_secondpass->SetUniformTexture3D ("VolumeTex", m_glsl_volume->GetTextureID (), 1);
	if (m_glsl_transfer_function) m_shader_secondpass->SetUniformTexture1D ("TransferFunc", m_glsl_transfer_function->GetTextureID (), 2);
	
	m_shader_secondpass->SetUniformInt ("ScreenSizeW", m_sdr_width);
	m_shader_secondpass->SetUniformInt ("ScreenSizeH", m_sdr_height);
	
	int volwidth = 1;
	int volheight = 1;
	int voldepth = 1;

	if (m_glsl_volume)
	{
		volwidth = m_glsl_volume->GetWidth ();
		volheight = m_glsl_volume->GetHeight ();
		voldepth = m_glsl_volume->GetDepth ();
	}
	m_shader_secondpass->SetUniformInt ("VolWidth", volwidth);
	m_shader_secondpass->SetUniformInt ("VolHeight", volheight);
	m_shader_secondpass->SetUniformInt ("VolDepth", voldepth);

	if (USE_DOUBLE_PRECISION)
	{


	}
	else
	{
		float geomWidth = 1.0f / m_cube_width;
		float geomHeight = 1.0f / m_cube_height;
		float geomDepth = 1.0f / m_cube_depth;

		m_shader_secondpass->SetUniformVector3f ("tex_scale", lqc::Vector3f (geomWidth / volwidth, geomHeight / volheight, geomDepth / voldepth));
	}



	if (USE_DOUBLE_PRECISION)
	{
		m_shader_secondpass->SetUniformDouble ("step_size", .5);
	}
	else
	{
		m_shader_secondpass->SetUniformFloat ("step_size", .05f);
	}
	
	gl::ExitOnGLError ("ERROR: Could not get shader uniform locations");

	gl::GLShader::Unbind ();

	m_svao = new gl::GLArrayObject (2);
	m_svao->Bind ();

	m_svbo = new gl::GLBufferObject (GL_ARRAY_BUFFER);
	m_sibo = new gl::GLBufferObject (GL_ELEMENT_ARRAY_BUFFER);

	//bind the VBO to the VAO
	m_svbo->SetBufferData (sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);

	m_svao->SetVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)0);
	m_svao->SetVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)sizeof(VERTICES[0].Position));

	//bind the IBO to the VAO
	m_sibo->SetBufferData (sizeof(INDICES), INDICES, GL_STATIC_DRAW);

	gl::GLArrayObject::Unbind ();
}

void RendererGLSL2P::ReloadVolume (vr::ScalarField* volume, bool resetslicesizes)
{
	printf("--------------------------------------------------\n");
	printf("RendererGLSL2P: Reload volume.\n");

	delete m_glsl_volume;
	m_glsl_volume = NULL; 

	if (resetslicesizes)
	{
		m_init_slice_x = 0; m_last_slice_x = volume->GetWidth ();
		m_init_slice_y = 0; m_last_slice_y = volume->GetHeight ();
		m_init_slice_z = 0; m_last_slice_z = volume->GetDepth ();
	}

	m_glsl_volume = vr::GenerateRTexture (volume, m_init_slice_x, m_init_slice_y, m_init_slice_z, m_last_slice_x, m_last_slice_y, m_last_slice_z);

	if (m_glsl_volume)
	{
		m_shader_secondpass->Bind ();
		m_shader_secondpass->SetUniformTexture3D ("VolumeTex", m_glsl_volume->GetTextureID (), 1);
		m_shader_secondpass->BindUniform ("VolumeTex");

		m_shader_secondpass->SetUniformInt ("VolWidth", m_glsl_volume->GetWidth ());
		m_shader_secondpass->BindUniform ("VolWidth");

		m_shader_secondpass->SetUniformInt ("VolHeight", m_glsl_volume->GetHeight ());
		m_shader_secondpass->BindUniform ("VolHeight");

		m_shader_secondpass->SetUniformInt ("VolDepth", m_glsl_volume->GetDepth ());
		m_shader_secondpass->BindUniform ("VolDepth");

		if (USE_DOUBLE_PRECISION)
		{
		}
		else
		{
			m_shader_secondpass->SetUniformVector3f ("tex_scale", lqc::Vector3f ((1.0f / m_cube_width) / (float)m_glsl_volume->GetWidth (),
				(1.0f / m_cube_height) / (float)m_glsl_volume->GetHeight (),
				(1.0f / m_cube_depth) / (float)m_glsl_volume->GetDepth ()));
		}
		m_shader_secondpass->BindUniform ("tex_scale");

		m_shader_secondpass->Unbind ();
		printf("RendererGLSL2P: Volume reloaded.\n");
		printf("--------------------------------------------------\n");
	}
}

void RendererGLSL2P::ReloadTransferFunction (vr::TransferFunction* tfunction)
{
	printf("--------------------------------------------------\n");
	printf("RendererGLSL2P: Reload TF.\n");

	delete m_glsl_transfer_function;
	m_glsl_transfer_function = NULL;
	
	if ( tfunction )
	{
		m_glsl_transfer_function = tfunction->GenerateTexture_1D_RGBA();

		if ( m_glsl_transfer_function )
		{
			m_shader_secondpass->Bind();

			m_shader_secondpass->SetUniformTexture1D("TransferFunc", m_glsl_transfer_function->GetTextureID(), 2);
			m_shader_secondpass->BindUniform("TransferFunc");

			//vr::ScalarField* vol = tfunction->GetVolume();
			//if (vol) {
			//  delete m_glsl_settex;
			//  m_glsl_settex = NULL;

			//  m_glsl_settex = new gl::GLTexture3D(m_last_slice_x - m_init_slice_x, m_last_slice_y - m_init_slice_y, m_last_slice_z - m_init_slice_z);
			//  if (m_glsl_settex) {
			//    m_glsl_settex->GenerateTexture(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
			//    m_glsl_settex->SetData(vol->GetVoxelSet(), GL_R32I, GL_RED_INTEGER, GL_INT);
			//    gl::ExitOnGLError("ERROR: After SetData");
			//    m_shader_secondpass->SetUniformTexture3D("SetTex", m_glsl_settex->GetTextureID(), 3);
			//    m_shader_secondpass->BindUniform("SetTex");
			//  }

			//  delete m_glsl_setqtdtex;
			//  m_glsl_setqtdtex = NULL;

			//  m_glsl_setqtdtex = new gl::GLTexture1D(vol->GetSetQtdSize());
			//  if (m_glsl_setqtdtex)
			//  {
			//    m_glsl_setqtdtex->GenerateTexture(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE);
			//    m_glsl_setqtdtex->SetData(vol->GetSetQtd(), GL_R32I, GL_RED_INTEGER, GL_INT);
			//    gl::ExitOnGLError("ERROR: After SetData");
			//    m_shader_secondpass->SetUniformTexture1D("SetQtdTex", m_glsl_setqtdtex->GetTextureID(), 4);
			//    m_shader_secondpass->BindUniform("SetQtdTex");
			//  }
			//}

			m_shader_secondpass->Unbind();
			printf("RendererGLSL2P: TF reloaded.\n");
			printf("--------------------------------------------------\n");
		}
	}
}

void RendererGLSL2P::ReloadVisibleSet(int set)
{
	m_shader_secondpass->Bind();
	m_shader_secondpass->SetUniformInt("VisibleSet", set);
	m_shader_secondpass->BindUniform("VisibleSet");
	m_shader_secondpass->Unbind();
}

void RendererGLSL2P::SetXRotation (float radius)
{
	m_x_rotation = radius;
}

float RendererGLSL2P::GetXRotation ()
{
	return m_x_rotation;
}

void RendererGLSL2P::SetYRotation (float radius)
{
	m_y_rotation = radius;
}

float RendererGLSL2P::GetYRotation ()
{
	return m_y_rotation;
}

void RendererGLSL2P::ReloadShaders ()
{
	m_shader_firstpass->Reload ();
	m_shader_secondpass->Reload ();
}

void RendererGLSL2P::ResetShaders (std::string shadername)
{
	gl::GLShader::Unbind ();
}

int RendererGLSL2P::GetInitXSlice ()
{
	return m_init_slice_x;
}

int RendererGLSL2P::GetLastXSlice ()
{
	return m_last_slice_x;
}

int RendererGLSL2P::GetInitYSlice ()
{
	return m_init_slice_y;
}

int RendererGLSL2P::GetLastYSlice ()
{
	return m_last_slice_y;
}

int RendererGLSL2P::GetInitZSlice ()
{
	return m_init_slice_z;
}

int RendererGLSL2P::GetLastZSlice ()
{
	return m_last_slice_z;
}

void RendererGLSL2P::SetSliceSizes (int ix, int lx, int iy, int ly, int iz, int lz)
{
	m_init_slice_x = ix;
	m_last_slice_x = lx;
	m_init_slice_y = iy;
	m_last_slice_y = ly;
	m_init_slice_z = iz;
	m_last_slice_z = lz;
}

void RendererGLSL2P::SetInitXSlice (int value)
{
	m_init_slice_x = value;
}

void RendererGLSL2P::SetLastXSlice (int value)
{
	m_last_slice_x = value;
}

void RendererGLSL2P::SetInitYSlice (int value)
{
	m_init_slice_y = value;
}

void RendererGLSL2P::SetLastYSlice (int value)
{
	m_last_slice_y = value;
}

void RendererGLSL2P::SetInitZSlice (int value)
{
	m_init_slice_z = value;
}

void RendererGLSL2P::SetLastZSlice (int value)
{
	m_last_slice_z = value;
}

void RendererGLSL2P::AutoModeling (vr::ScalarField* volume)
{
	if (m_glsl_volume)
	{
		int max = std::max (m_glsl_volume->GetWidth (), std::max (m_glsl_volume->GetHeight (), m_glsl_volume->GetDepth ()));
		m_cube_width  = (float)m_glsl_volume->GetWidth () / (float)max;
		m_cube_height = (float)m_glsl_volume->GetHeight () / (float)max;
		m_cube_depth  = (float)m_glsl_volume->GetDepth () / (float)max;
	}
	else
	{
		m_cube_width  = 1.0f;
		m_cube_height = 1.0f;
		m_cube_depth  = 1.0f;
	}

	printf ("Geometry: %f, %f, %f\n", m_cube_width, m_cube_height, m_cube_depth);
	m_shader_secondpass->Bind ();
	
	if (USE_DOUBLE_PRECISION)
	{
	}
	else
	{
		m_shader_secondpass->SetUniformVector3f ("tex_scale", lqc::Vector3f ((1.0f / m_cube_width) / (float)m_glsl_volume->GetWidth (),
			(1.0f / m_cube_height) / (float)m_glsl_volume->GetHeight (),
			(1.0f / m_cube_depth) / (float)m_glsl_volume->GetDepth ()));
	}
	m_shader_secondpass->BindUniform ("tex_scale");
	m_shader_secondpass->Unbind ();
}

void RendererGLSL2P::ApplyModeling (float xw, float yh, float zd)
{
	m_cube_width = xw;
	m_cube_height = yh;
	m_cube_depth = zd;
}

void RendererGLSL2P::SetCubeWidth (float w)
{
	m_cube_width = w;
}

void RendererGLSL2P::SetCubeHeight (float h)
{
	m_cube_height = h;
}

void RendererGLSL2P::SetCubeDepth (float z)
{
	m_cube_depth = z;
}

float RendererGLSL2P::GetCubeWidth ()
{
	return m_cube_width;
}

float RendererGLSL2P::GetCubeHeight ()
{
	return m_cube_height;
}

float RendererGLSL2P::GetCubeDepth ()
{
	return m_cube_depth;
}

void RendererGLSL2P::SaveInitCameraState (std::string filename)
{
	std::ofstream state_file;
	state_file.open (filename);
	if (state_file.is_open ())
	{
		state_file << "GLSL2D\n";
		state_file << 1 << "\n";

		state_file << m_x_rotation << "\n";
		state_file << m_y_rotation << "\n";

		state_file.close ();
	}
}

void RendererGLSL2P::LoadInitCameraState (std::string filename)
{
	std::ifstream state_file;
	state_file.open (filename);
	if (state_file.is_open ())
	{
		std::string method_name;
		std::getline (state_file, method_name);
		int view_method;
		state_file >> view_method;

		state_file >> m_x_rotation;
		state_file >> m_y_rotation;

		state_file.close ();
	}
}

void RendererGLSL2P::SetStepDistance (float step_distance)
{
	if (step_distance <= 0)
		step_distance = 10;

	m_shader_secondpass->Bind ();
	if (USE_DOUBLE_PRECISION)
		m_shader_secondpass->SetUniformFloat ("step_size", (double)step_distance);
	else
		m_shader_secondpass->SetUniformFloat ("step_size", step_distance);
	
	m_shader_secondpass->BindUniform ("step_size");
	gl::GLShader::Unbind ();
}

/*
void TimerFunction (int Value)
{
if (0 != Value) {
char* TempString = (char*)
malloc (512 + strlen (WINDOW_TITLE_PREFIX));

sprintf_s (
TempString
, 512 + strlen (WINDOW_TITLE_PREFIX)
, "%s: %d Frames Per Second @ %d x %d",
WINDOW_TITLE_PREFIX,
FrameCount * 4,
CurrentWidth,
CurrentHeight
);

glutSetWindowTitle (TempString);
free (TempString);
}

FrameCount = 0;
glutTimerFunc (250, TimerFunction, 1);
}
*/