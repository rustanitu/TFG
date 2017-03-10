#include "ERNRenderer.h"

#include "../gbuffer.h"

#include <atfg/ScalarField.h>
#include <atfg/TransferFunction.h>
#include <atfg/Utils.h>

#include <glutils/GLUtils.h>
#include <math/MUtils.h>
#include <lqc/lqcdefines.h>

ERNRenderer::ERNRenderer ()
: m_FrameCount (0),
m_LastTime (0),
m_x_rotation (0.0f),
m_y_rotation (0.0f),
m_glfbo (NULL),
m_render (0),
m_iterate (false),
m_glsl_volume (NULL),
m_glsl_gradient (NULL),
m_glsl_transfer_function (NULL)
{
  m_fvao = NULL; m_fvbo = NULL; m_fibo = NULL;
  m_svao = NULL; m_svbo = NULL; m_sibo = NULL;
  m_glsl_use_gradient = 0;
  m_glsl_samples = 64;

  m_cube_width = 1.0f;
  m_cube_height = 1.0f;
  m_cube_depth = 1.0f;

  m_init_slice_x = 0;
  m_last_slice_x = 0;
  m_init_slice_y = 0;
  m_last_slice_y = 0;
  m_init_slice_z = 0;
  m_last_slice_z = 0;

  glEnable (GL_AMD_sparse_texture);
  glEnable (GL_ARB_sparse_texture);
}

ERNRenderer::~ERNRenderer ()
{
  Destroy ();
}

void ERNRenderer::CreateScene (int CurrentWidth, int CurrentHeight, vr::ScalarField* volume, gl::GLTexture* tf, bool resetslices)
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

bool ERNRenderer::Render (int Width, int Height)
{
  ++m_FrameCount;

  clock_t Now = clock ();

  if (m_LastTime == 0)
    m_LastTime = Now;

  if (m_iterate)
  {
    m_y_rotation += 45.0f * ((float)(Now - m_LastTime) / CLOCKS_PER_SEC);
  }
  m_ModelMatrix = lqc::IDENTITY_MATRIX;
  lqc::TranslateMatrix (&m_ModelMatrix, -(m_cube_width / 2.0f), -(m_cube_height / 2.0f), -(m_cube_depth / 2.0f));
  lqc::RotateAboutX (&m_ModelMatrix, m_x_rotation * (float)PI / 180.0f);
  lqc::RotateAboutY (&m_ModelMatrix, m_y_rotation * (float)PI / 180.0f);

  m_glfbo->Bind ();
  FirstPass ();
  SecondPass ();

  m_glfbo->renderBuffer (Width, Height, 0);

  GBuffer::Unbind ();
  m_LastTime = Now;
  return true;
}

void ERNRenderer::Resize (int Width, int Height)
{
  m_sdr_width = Width;
  m_sdr_height = Height;
  glViewport (0, 0, Width, Height);
  m_ProjectionMatrix =
    lqc::CreateProjectionMatrix (
    45,
    (float)Width / (float)Height,
    1.0f,
    100.0f
    );

  m_shader_firstpass->Bind ();
  m_shader_firstpass->SetUniformMatrix4f ("ProjectionMatrix", m_ProjectionMatrix);
  m_shader_firstpass->BindUniform ("ProjectionMatrix");
  m_shader_firstpass->Unbind ();

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

void ERNRenderer::CreateFirstPass ()
{
  float wx = m_cube_width;
  float hy = m_cube_height;
  float dz = m_cube_depth;

  const ERNRenderer::Vertex VERTICES[8] =
  {
    { { 0.0f, 0.0f, dz }, { 0.0f, 0.0f, dz } },
    { { 0.0f, hy, dz }, { 0.0f, hy, dz } },
    { { wx, hy, dz }, { wx, hy, dz } },
    { { wx, 0.0f, dz }, { wx, 0.0f, dz } },
    { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } },
    { { 0.0f, hy, 0.0f }, { 0.0f, hy, 0.0f } },
    { { wx, hy, 0.0f }, { wx, hy, 0.0f } },
    { { wx, 0.0f, 0.0f }, { wx, 0.0f, 0.0f } }
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

  m_shader_firstpass = new gl::GLShader ("shader/StructuredDataset/ERN/backface.vert",
    "shader/StructuredDataset/ERN/backface.frag");
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

void ERNRenderer::DestroyCube ()
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

void ERNRenderer::DestroyQuad ()
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

void ERNRenderer::Destroy ()
{
  DestroyCube ();
  DestroyQuad ();
  gl::ExitOnGLError ("ERROR: Could not destroy the shaders");

  ////////////////////////////////
  if (m_glsl_volume)
    delete m_glsl_volume;
  m_glsl_volume = NULL;

  if (m_glsl_gradient)
    delete m_glsl_gradient;
  m_glsl_gradient = NULL;

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

void ERNRenderer::BindShaderUniforms ()
{
  m_shader_firstpass->Bind ();
  m_shader_firstpass->BindUniforms ();
  m_shader_firstpass->Unbind ();

  m_shader_secondpass->Bind ();
  m_shader_secondpass->BindUniforms ();
  m_shader_secondpass->Unbind ();
}

void ERNRenderer::ResetGeometry ()
{
  DestroyCube ();
  DestroyQuad ();

  CreateFirstPass ();
  CreateSecondPass ();

  BindShaderUniforms ();
}

void ERNRenderer::DrawCube (void)
{
  m_shader_firstpass->Bind ();

  m_fvao->DrawElements (GL_TRIANGLES, 36, GL_UNSIGNED_INT);

  gl::GLArrayObject::Unbind ();
  gl::GLShader::Unbind ();
}

//Draw a cube’s front faces into surface A and back faces into surface B. This determines ray intervals.
// Attach two textures to the current FBO to render to both surfaces simultaneously.
//Use a fragment shader that writes out normalized object-space coordinates to the RGB channels.
void ERNRenderer::FirstPass ()
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
void ERNRenderer::SecondPass ()
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

void ERNRenderer::CreateSecondPass ()
{
  float wx = m_cube_width;
  float hy = m_cube_height;
  float dz = m_cube_depth;

  const ERNRenderer::Vertex VERTICES[8] =
  {
    { { 0.0f, 0.0f, dz }, { 0.0f, 0.0f, dz } },
    { { 0.0f, hy, dz }, { 0.0f, hy, dz } },
    { { wx, hy, dz }, { wx, hy, dz } },
    { { wx, 0.0f, dz }, { wx, 0.0f, dz } },
    { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } },
    { { 0.0f, hy, 0.0f }, { 0.0f, hy, 0.0f } },
    { { wx, hy, 0.0f }, { wx, hy, 0.0f } },
    { { wx, 0.0f, 0.0f }, { wx, 0.0f, 0.0f } }
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

  m_shader_secondpass = new gl::GLShader ("shader/StructuredDataset/ERN/raycasting.vert",
    "shader/StructuredDataset/ERN/raycasting.frag");
  m_shader_secondpass->SetUniformMatrix4f ("ModelMatrix", lqc::IDENTITY_MATRIX);
  m_shader_secondpass->SetUniformMatrix4f ("ViewMatrix", lqc::IDENTITY_MATRIX);
  m_shader_secondpass->SetUniformMatrix4f ("ProjectionMatrix", lqc::IDENTITY_MATRIX);

  m_shader_secondpass->SetUniformTexture2D ("ExitPoints", m_glfbo->m_backtexture, 0);
  if (m_glsl_volume) m_shader_secondpass->SetUniformTexture3D ("VolumeTex", m_glsl_volume->GetTextureID (), 1);
  if (m_glsl_transfer_function) m_shader_secondpass->SetUniformTexture1D ("TransferFunc", m_glsl_transfer_function->GetTextureID (), 2);

  m_shader_secondpass->SetUniformInt ("ScreenSizeW", m_sdr_width);
  m_shader_secondpass->SetUniformInt ("ScreenSizeH", m_sdr_height);

  if (m_glsl_volume)
  {
    m_shader_secondpass->SetUniformInt ("VolWidth", m_glsl_volume->GetWidth ());
    m_shader_secondpass->SetUniformInt ("VolHeight", m_glsl_volume->GetHeight ());
    m_shader_secondpass->SetUniformInt ("VolDepth", m_glsl_volume->GetDepth ());
  }
  else
  {
    m_shader_secondpass->SetUniformInt ("VolWidth", 1);
    m_shader_secondpass->SetUniformInt ("VolHeight", 1);
    m_shader_secondpass->SetUniformInt ("VolDepth", 1);
  }

  m_shader_secondpass->SetUniformFloat ("GeomVolWidth", m_cube_width);
  m_shader_secondpass->SetUniformFloat ("GeomVolHeight", m_cube_height);
  m_shader_secondpass->SetUniformFloat ("GeomVolDepth", m_cube_depth);

  m_shader_secondpass->SetUniformFloat ("step_size", 1.f);
  m_shader_secondpass->SetUniformInt ("render_mode", 0);

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

void ERNRenderer::ReloadVolume (vr::ScalarField* volume, bool resetslicesizes)
{
  if (m_glsl_volume) delete m_glsl_volume;
  m_glsl_volume = NULL;
  if (m_glsl_gradient) delete m_glsl_gradient;
  m_glsl_gradient = NULL;

  if (resetslicesizes)
  {
    m_init_slice_x = 0; m_last_slice_x = volume->GetWidth () > 512 ? 512 : volume->GetWidth ();
    m_init_slice_y = 0; m_last_slice_y = volume->GetHeight () > 512 ? 512 : volume->GetHeight ();
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

    m_shader_secondpass->Unbind ();
  }
  //ResetUseGradient (m_glsl_use_gradient, volume);
}

void ERNRenderer::ReloadTransferFunction (vr::TransferFunction* tfunction)
{
  if (m_glsl_transfer_function) delete m_glsl_transfer_function;
  m_glsl_transfer_function = NULL;

  m_glsl_transfer_function = tfunction->GenerateTexture_RGBA ();

  if (m_glsl_transfer_function)
  {
    m_shader_secondpass->Bind ();
    m_shader_secondpass->SetUniformTexture1D ("TransferFunc", m_glsl_transfer_function->GetTextureID (), 2);
    m_shader_secondpass->BindUniform ("TransferFunc");
    m_shader_secondpass->Unbind ();
  }
}

void ERNRenderer::ResetUseGradient (int use_gradient, vr::ScalarField* volume)
{
  //if (m_glsl_gradient == NULL)
  //{
  //  m_glsl_gradient = volume->GenerateGradientTexture (m_init_slice_x, m_init_slice_y, m_init_slice_z, m_last_slice_x, m_last_slice_y, m_last_slice_z);
  //  m_shader_secondpass->SetUniformTexture3D ("gradientTexture", m_glsl_gradient->GetTextureID (), 4);
  //}
  //
  //m_glsl_use_gradient = use_gradient;
  //m_shader_secondpass->SetUniformInt ("usegradient", m_glsl_use_gradient);
}

void ERNRenderer::ResetSamples (int samples)
{
  if (samples >= 1)
  {
    m_glsl_samples = samples;
  }
}

void ERNRenderer::SetXRotation (float radius)
{
  m_x_rotation = radius;
}

float ERNRenderer::GetXRotation ()
{
  return m_x_rotation;
}

void ERNRenderer::SetYRotation (float radius)
{
  m_y_rotation = radius;
}

float ERNRenderer::GetYRotation ()
{
  return m_y_rotation;
}

void ERNRenderer::ReloadShaders ()
{
  m_shader_firstpass->Reload ();
  m_shader_secondpass->Reload ();
}

void ERNRenderer::ResetShaders (std::string shadername)
{
  gl::GLShader::Unbind ();
}

int ERNRenderer::GetInitXSlice ()
{
  return m_init_slice_x;
}

int ERNRenderer::GetLastXSlice ()
{
  return m_last_slice_x;
}

int ERNRenderer::GetInitYSlice ()
{
  return m_init_slice_y;
}

int ERNRenderer::GetLastYSlice ()
{
  return m_last_slice_y;
}

int ERNRenderer::GetInitZSlice ()
{
  return m_init_slice_z;
}

int ERNRenderer::GetLastZSlice ()
{
  return m_last_slice_z;
}

void ERNRenderer::SetSliceSizes (int ix, int lx, int iy, int ly, int iz, int lz)
{
  m_init_slice_x = ix;
  m_last_slice_x = lx;
  m_init_slice_y = iy;
  m_last_slice_y = ly;
  m_init_slice_z = iz;
  m_last_slice_z = lz;
}

void ERNRenderer::SetInitXSlice (int value)
{
  m_init_slice_x = value;
}

void ERNRenderer::SetLastXSlice (int value)
{
  m_last_slice_x = value;
}

void ERNRenderer::SetInitYSlice (int value)
{
  m_init_slice_y = value;
}

void ERNRenderer::SetLastYSlice (int value)
{
  m_last_slice_y = value;
}

void ERNRenderer::SetInitZSlice (int value)
{
  m_init_slice_z = value;
}

void ERNRenderer::SetLastZSlice (int value)
{
  m_last_slice_z = value;
}

void ERNRenderer::AutoModeling (vr::ScalarField* volume)
{
  if (m_glsl_volume)
  {
    int max = std::max (m_glsl_volume->GetWidth (), std::max (m_glsl_volume->GetHeight (), m_glsl_volume->GetDepth ()));
    m_cube_width = (float)m_glsl_volume->GetWidth () / (float)max;
    m_cube_height = (float)m_glsl_volume->GetHeight () / (float)max;
    m_cube_depth = (float)m_glsl_volume->GetDepth () / (float)max;
  }
  else
  {
    m_cube_width = 1.0f;
    m_cube_height = 1.0f;
    m_cube_depth = 1.0f;
  }

  printf ("Geometry: %f, %f, %f\n", m_cube_width, m_cube_height, m_cube_depth);
  m_shader_secondpass->Bind ();
  m_shader_secondpass->SetUniformFloat ("GeomVolWidth", m_cube_width);
  m_shader_secondpass->SetUniformFloat ("GeomVolHeight", m_cube_height);
  m_shader_secondpass->SetUniformFloat ("GeomVolDepth", m_cube_depth);
  m_shader_secondpass->BindUniform ("GeomVolWidth");
  m_shader_secondpass->BindUniform ("GeomVolHeight");
  m_shader_secondpass->BindUniform ("GeomVolDepth");
  m_shader_secondpass->Unbind ();
}

void ERNRenderer::ApplyModeling (float xw, float yh, float zd)
{
  m_cube_width = xw;
  m_cube_height = yh;
  m_cube_depth = zd;
}

void ERNRenderer::SetCubeWidth (float w)
{
  m_cube_width = w;
}

void ERNRenderer::SetCubeHeight (float h)
{
  m_cube_height = h;
}

void ERNRenderer::SetCubeDepth (float z)
{
  m_cube_depth = z;
}

float ERNRenderer::GetCubeWidth ()
{
  return m_cube_width;
}

float ERNRenderer::GetCubeHeight ()
{
  return m_cube_height;
}

float ERNRenderer::GetCubeDepth ()
{
  return m_cube_depth;
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