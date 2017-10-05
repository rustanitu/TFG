#include "RendererIAS.h"

#include "../gbuffer.h"

#include <atfg/Volume.h>
#include <atfg/TransferFunction.h>
#include <glutils/GLUtils.h>
#include <math/MUtils.h>
#include <lqc/lqcdefines.h>

#include <atfg/Utils.h>

#include <fstream>

RendererIAS::RendererIAS ()
: m_FrameCount (0),
m_LastTime (0),
m_x_rotation (0.0f),
m_y_rotation (0.0f),
m_glfbo (NULL),
m_glsl_volume (NULL),
m_glsl_gradient (NULL),
m_glsl_transfer_function (NULL),
USE_DOUBLE_PRECISION (false)
{
  m_fvao = NULL; m_fvbo = NULL; m_fibo = NULL;
  m_svao = NULL; m_svbo = NULL; m_sibo = NULL;
  m_glsl_use_gradient = 0;

  m_cube_width = 1.0f;
  m_cube_height = 1.0f;
  m_cube_depth = 1.0f;

  m_init_slice_x = 0;
  m_last_slice_x = 0;
  m_init_slice_y = 0;
  m_last_slice_y = 0;
  m_init_slice_z = 0;
  m_last_slice_z = 0;

  m_initial_step = 0.5f;
  m_tol_error = 10.0f;

  SHADER_ID = 0;
}

RendererIAS::~RendererIAS ()
{
  Destroy ();
}

void RendererIAS::CreateScene (int CurrentWidth, int CurrentHeight, vr::Volume* volume, gl::GLTexture* tf, bool resetslices)
{
  m_sdr_width = CurrentWidth;
  m_sdr_height = CurrentHeight;
  m_ModelMatrix = lqc::IDENTITY_MATRIX;
  m_ProjectionMatrix = lqc::IDENTITY_MATRIX;
  m_ViewMatrix = lqc::IDENTITY_MATRIX;
  float zcam = -2.f;
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

bool RendererIAS::Render (int Width, int Height)
{
  ++m_FrameCount;

  clock_t Now = clock ();

  if (m_LastTime == 0)
    m_LastTime = Now;

  m_ModelMatrix = lqc::IDENTITY_MATRIX;
  lqc::TranslateMatrix (&m_ModelMatrix, -(m_cube_width / 2.0f), -(m_cube_height / 2.0f), -(m_cube_depth / 2.0f));
  lqc::RotateAboutX (&m_ModelMatrix, m_x_rotation * (float)PI / 180.0f);
  lqc::RotateAboutY (&m_ModelMatrix, m_y_rotation * (float)PI / 180.0f);
  //lqc::TranslateMatrix (&m_ModelMatrix, 0, 0, 0.2);


  m_shader_firstpass->Bind ();
  m_shader_firstpass->SetUniformMatrix4f ("ModelMatrix", m_ModelMatrix);
  m_shader_firstpass->SetUniformMatrix4f ("ViewMatrix", m_ViewMatrix);
  m_shader_firstpass->BindUniform ("ModelMatrix");
  m_shader_firstpass->BindUniform ("ViewMatrix");
  gl::GLShader::Unbind ();

  m_glfbo->Bind ();
  FirstPass ();
  SecondPass ();
  
  m_glfbo->renderBuffer (Width, Height, 0);

  GBuffer::Unbind ();
  m_LastTime = Now;
  return true;
}

void RendererIAS::Resize (int Width, int Height)
{
  m_sdr_width = Width;
  m_sdr_height = Height;
  glViewport (0, 0, Width, Height);
  m_ProjectionMatrix = 
    //lqc::CreateOrthoMatrix (0.0f, (float)Width, (float)Height, 0.0f, 1.0f, 500.0f)
    //lqc::CreateOrthoMatrix (-1.0f, 1.0f, -1.0f, 1.0f, -5.0f, 5.0f)
    lqc::CreateProjectionMatrix (45, (float)Width / (float)Height, 1.0f, 100.0f)
    ;

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

void RendererIAS::CreateFirstPass ()
{
  float wx = m_cube_width;
  float hy = m_cube_height;
  float dz = m_cube_depth;

  const RendererIAS::Vertex VERTICES[8] =
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

void RendererIAS::DestroyCube ()
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

void RendererIAS::DestroyQuad ()
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

void RendererIAS::Destroy ()
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

void RendererIAS::BindShaderUniforms ()
{
  m_shader_firstpass->Bind ();
  m_shader_firstpass->BindUniforms ();
  m_shader_firstpass->Unbind ();

  m_shader_secondpass->Bind ();
  m_shader_secondpass->BindUniforms ();
  m_shader_secondpass->Unbind ();
}

void RendererIAS::ResetGeometry ()
{
  DestroyCube ();
  DestroyQuad ();

  CreateFirstPass ();
  CreateSecondPass ();

  BindShaderUniforms ();
}

void RendererIAS::DrawCube (void)
{
  m_shader_firstpass->Bind ();

  m_fvao->DrawElements (GL_TRIANGLES, 36, GL_UNSIGNED_INT);

  gl::GLArrayObject::Unbind ();
  gl::GLShader::Unbind ();
}

void RendererIAS::SetRealTolerance (float tol)
{  
  m_tol_error = tol;

  m_shader_secondpass->Bind ();
  if (USE_DOUBLE_PRECISION)
    m_shader_secondpass->SetUniformDouble ("tol_error", 15.0 * (double)tol);
  else
    m_shader_secondpass->SetUniformFloat ("tol_error", 15.0f * tol);

  m_shader_secondpass->BindUniform ("tol_error");
  m_shader_secondpass->Unbind ();
}

//Draw a cube’s front faces into surface A and back faces into surface B. This determines ray intervals.
// Attach two textures to the current FBO to render to both surfaces simultaneously.
//Use a fragment shader that writes out normalized object-space coordinates to the RGB channels.
void RendererIAS::FirstPass ()
{
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
void RendererIAS::SecondPass ()
{
  m_shader_secondpass->Bind ();
  m_shader_secondpass->SetUniformMatrix4f ("ModelMatrix", m_ModelMatrix);
  m_shader_secondpass->SetUniformMatrix4f ("ViewMatrix", m_ViewMatrix);
  m_shader_secondpass->BindUniform ("ModelMatrix");
  m_shader_secondpass->BindUniform ("ViewMatrix");
  m_shader_secondpass->BindUniform ("txt2D_exitpoints");
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

void RendererIAS::CreateSecondPass ()
{
  float wx = m_cube_width;
  float hy = m_cube_height;
  float dz = m_cube_depth;

  const RendererIAS::Vertex VERTICES[8] =
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


  if (USE_DOUBLE_PRECISION)
    if (SHADER_ID == 0)
      m_shader_secondpass = new gl::GLShader ("shader/StructuredDataset/raycasting.vert",
        "shader/StructuredDataset/IAS/d_Simpson_Half_Half.frag");
    else if (SHADER_ID == 1)
      m_shader_secondpass = new gl::GLShader ("shader/StructuredDataset/raycasting.vert",
        "shader/StructuredDataset/IAS/d_Simpson_Half_Queue.frag");
    else if (SHADER_ID == 2)
      m_shader_secondpass = new gl::GLShader ("shader/StructuredDataset/raycasting.vert",
        "shader/StructuredDataset/IAS/d_Simpson_Half_Queue_acc.frag");
    else
      m_shader_secondpass = new gl::GLShader ("shader/StructuredDataset/raycasting.vert",
      "shader/StructuredDataset/IAS/d_Simpson_Half_Iteration.frag");
  else
    if (SHADER_ID == 0)
      m_shader_secondpass = new gl::GLShader ("shader/StructuredDataset/raycasting.vert",
        "shader/StructuredDataset/IterativeAdaptiveSimpson/f_coupled.frag");
    else if (SHADER_ID == 1)
      m_shader_secondpass = new gl::GLShader ("shader/StructuredDataset/raycasting.vert",
      "shader/StructuredDataset/IterativeAdaptiveSimpson/f_decoupled.frag");

  m_shader_secondpass->SetUniformMatrix4f ("ModelMatrix", lqc::IDENTITY_MATRIX);
  m_shader_secondpass->SetUniformMatrix4f ("ViewMatrix", lqc::IDENTITY_MATRIX);
  m_shader_secondpass->SetUniformMatrix4f ("ProjectionMatrix", lqc::IDENTITY_MATRIX);
  
  m_shader_secondpass->SetUniformTexture2D ("txt2D_exitpoints", m_glfbo->m_backtexture, 0);
  if (m_glsl_volume) m_shader_secondpass->SetUniformTexture3D ("txt3D_volume", m_glsl_volume->GetTextureID (), 1);
  if (m_glsl_transfer_function) m_shader_secondpass->SetUniformTexture1D ("txt1D_transferfunction", m_glsl_transfer_function->GetTextureID (), 2);
  
  m_shader_secondpass->SetUniformInt ("ScreenSizeW", m_sdr_width);
  m_shader_secondpass->SetUniformInt ("ScreenSizeH", m_sdr_height);
  

  int volwidth  = 1;
  int volheight = 1;
  int voldepth  = 1;

  if (m_glsl_volume)
  {
    volwidth  = m_glsl_volume->GetWidth  ();
    volheight = m_glsl_volume->GetHeight ();
    voldepth  = m_glsl_volume->GetDepth  ();
  }
  m_shader_secondpass->SetUniformInt ("VolWidth" , volwidth);
  m_shader_secondpass->SetUniformInt ("VolHeight", volheight);
  m_shader_secondpass->SetUniformInt ("VolDepth" , voldepth);

  if (USE_DOUBLE_PRECISION)
  {
    m_shader_secondpass->SetUniformDouble ("initial_step", (double)m_initial_step);
    m_shader_secondpass->SetUniformDouble ("tol_error", 15.0 * (double)m_tol_error);
  }
  else
  {
    m_shader_secondpass->SetUniformFloat ("initial_step", m_initial_step);
    m_shader_secondpass->SetUniformFloat ("tol_error", 15.0f * m_tol_error);
  }

  if (USE_DOUBLE_PRECISION)
  {


  }
  else
  {
    float geomWidth  = 1.0f / m_cube_width;
    float geomHeight = 1.0f / m_cube_height;
    float geomDepth  = 1.0f / m_cube_depth;

    m_shader_secondpass->SetUniformVector3f ("tex_scale", glm::vec3(geomWidth  / volwidth, geomHeight / volheight, geomDepth  / voldepth));
  }

  m_shader_secondpass->SetUniformFloat ("hmin", 0.1);
  m_shader_secondpass->SetUniformFloat ("hmax", 2.0);

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

void RendererIAS::ReloadVolume (vr::Volume* volume, bool resetslicesizes)
{
  if (m_glsl_volume) delete m_glsl_volume;
  m_glsl_volume = NULL; 
  if (m_glsl_gradient) delete m_glsl_gradient;
  m_glsl_gradient = NULL;

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
    m_shader_secondpass->SetUniformTexture3D ("txt3D_volume", m_glsl_volume->GetTextureID (), 1);
    m_shader_secondpass->BindUniform ("txt3D_volume");

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
      m_shader_secondpass->SetUniformVector3f ("tex_scale", glm::vec3((1.0f / m_cube_width) / (float)m_glsl_volume->GetWidth (),
                                                                          (1.0f / m_cube_height) / (float)m_glsl_volume->GetHeight (),
                                                                          (1.0f / m_cube_depth) / (float)m_glsl_volume->GetDepth ()));
    }
    m_shader_secondpass->BindUniform ("tex_scale");

    m_shader_secondpass->Unbind ();
  }
  //ResetUseGradient (m_glsl_use_gradient, volume);
}

void RendererIAS::ReloadTransferFunction (vr::TransferFunction* tfunction)
{
  if (m_glsl_transfer_function) delete m_glsl_transfer_function;
  m_glsl_transfer_function = NULL;
  
  m_glsl_transfer_function = tfunction->GenerateTexture_RGBA ();

  if (m_glsl_transfer_function)
  {
    m_shader_secondpass->Bind ();
    m_shader_secondpass->SetUniformTexture1D ("txt1D_transferfunction", m_glsl_transfer_function->GetTextureID (), 2);
    m_shader_secondpass->BindUniform ("txt1D_transferfunction");
    m_shader_secondpass->Unbind ();
  }
}

void RendererIAS::ResetUseGradient (int use_gradient, vr::Volume* volume)
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

void RendererIAS::SetXRotation (float radius)
{
  m_x_rotation = radius;
}

float RendererIAS::GetXRotation ()
{
  return m_x_rotation;
}

void RendererIAS::SetYRotation (float radius)
{
  m_y_rotation = radius;
}

float RendererIAS::GetYRotation ()
{
  return m_y_rotation;
}

void RendererIAS::ReloadShaders ()
{
  m_shader_firstpass->Reload ();
  m_shader_secondpass->Reload ();
}

void RendererIAS::ResetShaders (std::string shadername)
{
  gl::GLShader::Unbind ();
}

int RendererIAS::GetInitXSlice ()
{
  return m_init_slice_x;
}

int RendererIAS::GetLastXSlice ()
{
  return m_last_slice_x;
}

int RendererIAS::GetInitYSlice ()
{
  return m_init_slice_y;
}

int RendererIAS::GetLastYSlice ()
{
  return m_last_slice_y;
}

int RendererIAS::GetInitZSlice ()
{
  return m_init_slice_z;
}

int RendererIAS::GetLastZSlice ()
{
  return m_last_slice_z;
}

void RendererIAS::SetSliceSizes (int ix, int lx, int iy, int ly, int iz, int lz)
{
  m_init_slice_x = ix;
  m_last_slice_x = lx;
  m_init_slice_y = iy;
  m_last_slice_y = ly;
  m_init_slice_z = iz;
  m_last_slice_z = lz;
}

void RendererIAS::SetInitXSlice (int value)
{
  m_init_slice_x = value;
}

void RendererIAS::SetLastXSlice (int value)
{
  m_last_slice_x = value;
}

void RendererIAS::SetInitYSlice (int value)
{
  m_init_slice_y = value;
}

void RendererIAS::SetLastYSlice (int value)
{
  m_last_slice_y = value;
}

void RendererIAS::SetInitZSlice (int value)
{
  m_init_slice_z = value;
}

void RendererIAS::SetLastZSlice (int value)
{
  m_last_slice_z = value;
}

void RendererIAS::AutoModeling (vr::Volume* volume)
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

  m_shader_secondpass->Bind ();
  printf ("Geometry: %f %f %f\n", m_cube_width, m_cube_height, m_cube_depth);

  if (USE_DOUBLE_PRECISION)
  {
  }
  else
  {
    m_shader_secondpass->SetUniformVector3f ("tex_scale", glm::vec3((1.0f / m_cube_width) / (float)m_glsl_volume->GetWidth (),
                                                                        (1.0f / m_cube_height) / (float)m_glsl_volume->GetHeight (),
                                                                        (1.0f / m_cube_depth) / (float)m_glsl_volume->GetDepth ()));
  }
  m_shader_secondpass->BindUniform ("tex_scale");
  m_shader_secondpass->Unbind ();
}

void RendererIAS::ApplyModeling (float xw, float yh, float zd)
{
  m_cube_width = xw;
  m_cube_height = yh;
  m_cube_depth = zd;
}

void RendererIAS::SetCubeWidth (float w)
{
  m_cube_width = w;
}

void RendererIAS::SetCubeHeight (float h)
{
  m_cube_height = h;
}

void RendererIAS::SetCubeDepth (float z)
{
  m_cube_depth = z;
}

float RendererIAS::GetCubeWidth ()
{
  return m_cube_width;
}

float RendererIAS::GetCubeHeight ()
{
  return m_cube_height;
}

float RendererIAS::GetCubeDepth ()
{
  return m_cube_depth;
}

void RendererIAS::SaveInitCameraState (std::string filename)
{
  std::ofstream state_file;
  state_file.open (filename);
  if (state_file.is_open ())
  {
    state_file << "IAS\n";
    state_file << 1 << "\n";

    state_file << m_x_rotation << "\n";
    state_file << m_y_rotation << "\n";

    state_file.close ();
  }
}

void RendererIAS::LoadInitCameraState (std::string filename)
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
