#include "GLSLRenderer.h"

#include "../gbuffer.h"

#include <volrend/Volume.h>
#include <volrend/TransferFunction.h>
#include <glutils/GLUtils.h>
#include <math/MUtils.h>
#include <lqc/lqcdefines.h>

GLSLRenderer::GLSLRenderer ()
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
  m_shader_name = "Riemann";

  m_cube_width = 1.0f;
  m_cube_height = 1.0f;
  m_cube_depth = 1.0f;

  m_init_slice_x = 0;
  m_last_slice_x = 0;
  m_init_slice_y = 0;
  m_last_slice_y = 0;
  m_init_slice_z = 0;
  m_last_slice_z = 0;
}

GLSLRenderer::~GLSLRenderer ()
{
  Destroy ();
}

void GLSLRenderer::CreateScene (int CurrentWidth, int CurrentHeight, vr::Volume* volume, lqc::GLTexture1D* tf, bool resetslices)
{
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
 
  CreateCube ();
  CreateQuad ();

  Resize (CurrentWidth, CurrentHeight);
}

bool GLSLRenderer::Render (int Width, int Height)
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
  
  if (m_render == 0)
    m_glfbo->renderBuffer (Width, Height, 3);
  else
    m_glfbo->renderBuffers (Width, Height);

  GBuffer::Unbind ();
  m_LastTime = Now;
  return true;
}

void GLSLRenderer::Resize (int Width, int Height)
{
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

  m_glfbo->resize (Width, Height);
}

void GLSLRenderer::CreateCube ()
{
  float wx = m_cube_width;
  float hy = m_cube_height;
  float dz = m_cube_depth;

  const GLSLRenderer::Vertex VERTICES[8] =
  {
    { { 0.0f, 0.0f,   dz, 1.0f }, { 0.0f, 0.0f,   dz, 1.0f } },
    { { 0.0f,   hy,   dz, 1.0f }, { 0.0f,   hy,   dz, 1.0f } },
    { {   wx,   hy,   dz, 1.0f }, {   wx,   hy,   dz, 1.0f } },
    { {   wx, 0.0f,   dz, 1.0f }, {   wx, 0.0f,   dz, 1.0f } },
    { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f } },
    { { 0.0f,   hy, 0.0f, 1.0f }, { 0.0f,   hy, 0.0f, 1.0f } },
    { {   wx,   hy, 0.0f, 1.0f }, {   wx,   hy, 0.0f, 1.0f } },
    { {   wx, 0.0f, 0.0f, 1.0f }, {   wx, 0.0f, 0.0f, 1.0f } }
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

  std::string snamevert = "";
  std::string snamefrag = "";
  
  std::string initpath = "shader/StructuredDataset/GLSL/";
  std::string lastpath = "1.";

  snamevert.append (initpath);
  snamevert.append (m_shader_name);
  snamevert.append (lastpath);
  snamevert.append ("vert");

  snamefrag.append (initpath);
  snamefrag.append (m_shader_name);
  snamefrag.append (lastpath);
  snamefrag.append ("frag");

  m_shader_firstpass = new lqc::GLShader (snamevert, snamefrag);
  m_shader_firstpass->SetUniformMatrix4f ("ModelMatrix", lqc::IDENTITY_MATRIX);
  m_shader_firstpass->SetUniformMatrix4f ("ViewMatrix", lqc::IDENTITY_MATRIX);
  m_shader_firstpass->SetUniformMatrix4f ("ProjectionMatrix", lqc::IDENTITY_MATRIX);
  lqc::ExitOnGLError ("ERROR: Could not get shader uniform locations");

  lqc::GLShader::Unbind ();

  m_fvao = new lqc::GLArrayObject (2);
  m_fvao->Bind ();

  m_fvbo = new lqc::GLBufferObject (lqc::GLBufferObject::TYPES::VERTEXBUFFEROBJECT);
  m_fibo = new lqc::GLBufferObject (lqc::GLBufferObject::TYPES::INDEXBUFFEROBJECT);

  //bind the VBO to the VAO
  m_fvbo->SetBufferData (sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);

  m_fvao->SetVertexAttribPointer (0, 4, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)0);
  m_fvao->SetVertexAttribPointer (1, 4, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)sizeof(VERTICES[0].Position));

  //bind the IBO to the VAO
  m_fibo->SetBufferData (sizeof(INDICES), INDICES, GL_STATIC_DRAW);

  lqc::GLArrayObject::Unbind ();
}

void GLSLRenderer::DestroyCube ()
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

void GLSLRenderer::CreateQuad ()
{
  const GLSLRenderer::QuadVertex VERTICES[4] =
  {
    { { -1.0f, -1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f } },
    { { 1.0f, -1.0f, -1.0f, 1.0f }, { 1.0f, 0.0f } },
    { { 1.0f, 1.0f, -1.0f, 1.0f }, { 1.0f, 1.0f } },
    { { -1.0f, 1.0f, -1.0f, 1.0f }, { 0.0f, 1.0f } }
  };

  const GLuint INDICES[6] =
  {
    0, 1, 2, 2, 3, 0
  };

  std::string snamevert = "";
  std::string snamefrag = "";

  std::string initpath = "shader/StructuredDataset/GLSL/";
  std::string lastpath = "2.";

  snamevert.append (initpath);
  snamevert.append (m_shader_name);
  snamevert.append (lastpath);
  snamevert.append ("vert");

  snamefrag.append (initpath);
  snamefrag.append (m_shader_name);
  snamefrag.append (lastpath);
  snamefrag.append ("frag");

  m_shader_secondpass = new lqc::GLShader (snamevert, snamefrag);
  m_shader_secondpass->SetUniformMatrix4f ("ModelMatrix", lqc::IDENTITY_MATRIX);
  m_shader_secondpass->SetUniformMatrix4f ("ViewMatrix", lqc::IDENTITY_MATRIX);
  m_shader_secondpass->SetUniformMatrix4f ("ProjectionMatrix", lqc::IDENTITY_MATRIX);
  m_shader_secondpass->SetUniformTexture2D ("backTexture", m_glfbo->m_backtexture, 0);
  m_shader_secondpass->SetUniformTexture2D ("frontTexture", m_glfbo->m_colortexture, 1);
  if (m_glsl_volume) m_shader_secondpass->SetUniformTexture3D ("volumeTexture", m_glsl_volume->GetTextureID (), 2);
  if (m_glsl_transfer_function) m_shader_secondpass->SetUniformTexture1D ("transferFunctionTexture", m_glsl_transfer_function->GetTextureID (), 3);
  if (m_glsl_gradient) m_shader_secondpass->SetUniformTexture3D ("gradientTexture", m_glsl_gradient->GetTextureID (), 4);
  m_shader_secondpass->SetUniformInt ("usegradient", m_glsl_use_gradient);
  m_shader_secondpass->SetUniformInt ("samples", m_glsl_samples);
  m_shader_secondpass->SetUniformFloat ("cube_width", m_cube_width);
  m_shader_secondpass->SetUniformFloat ("cube_height", m_cube_height);
  m_shader_secondpass->SetUniformFloat ("cube_depth", m_cube_depth);

  lqc::ExitOnGLError ("ERROR: Could not get shader uniform locations");

  lqc::GLShader::Unbind ();

  m_svao = new lqc::GLArrayObject (2);
  m_svao->Bind ();

  m_svbo = new lqc::GLBufferObject (GL_ARRAY_BUFFER);
  m_sibo = new lqc::GLBufferObject (GL_ELEMENT_ARRAY_BUFFER);

  //bind the VBO to the VAO
  m_svbo->SetBufferData (sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);

  m_svao->SetVertexAttribPointer (0, 4, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)0);
  m_svao->SetVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)sizeof(VERTICES[0].Position));

  //bind the IBO to the VAO
  m_sibo->SetBufferData (sizeof(INDICES), INDICES, GL_STATIC_DRAW);

  lqc::GLArrayObject::Unbind ();
}

void GLSLRenderer::DestroyQuad ()
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

void GLSLRenderer::Destroy ()
{
  DestroyCube ();
  DestroyQuad ();
  lqc::ExitOnGLError ("ERROR: Could not destroy the shaders");

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

void GLSLRenderer::ResetGeometry ()
{
  DestroyCube ();
  DestroyQuad ();

  CreateCube ();
  CreateQuad ();
}

void GLSLRenderer::DrawCube (void)
{
  m_shader_firstpass->Bind ();

  m_fvao->DrawElements (GL_TRIANGLES, 36, GL_UNSIGNED_INT);

  lqc::GLArrayObject::Unbind ();
  lqc::GLShader::Unbind ();
}

//Draw a cube’s front faces into surface A and back faces into surface B. This determines ray intervals.
// Attach two textures to the current FBO to render to both surfaces simultaneously.
//Use a fragment shader that writes out normalized object-space coordinates to the RGB channels.
void GLSLRenderer::FirstPass ()
{
  m_shader_firstpass->Bind ();

  m_shader_firstpass->SetUniformMatrix4f ("ModelMatrix", m_ModelMatrix);
  m_shader_firstpass->SetUniformMatrix4f ("ViewMatrix", m_ViewMatrix);
  m_shader_firstpass->BindUniform ("ModelMatrix");
  m_shader_firstpass->BindUniform ("ViewMatrix");

  //draw front faces
  //draw the pixel positions to the texture
  {
    glCullFace (GL_BACK);
    GLuint attachments[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers (1, attachments);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    DrawCube ();
  }

  //draw back faces
  //draw the pixel positions to the texture
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
void GLSLRenderer::SecondPass ()
{
  glCullFace (GL_BACK);
  GLuint attachments[2] = { GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
  glDrawBuffers (2, attachments);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  m_shader_secondpass->Bind ();

  m_shader_secondpass->BindUniform ("backTexture");
  m_shader_secondpass->BindUniform ("frontTexture");
  m_shader_secondpass->BindUniform ("volumeTexture");
  m_shader_secondpass->BindUniform ("transferFunctionTexture");
  m_shader_secondpass->BindUniform ("gradientTexture");
  m_shader_secondpass->BindUniform ("usegradient");
  m_shader_secondpass->BindUniform ("samples");

  m_shader_secondpass->BindUniform ("ModelMatrix");
  m_shader_secondpass->BindUniform ("ViewMatrix");
  m_shader_secondpass->BindUniform ("ProjectionMatrix");

  m_shader_secondpass->BindUniform ("cube_width");
  m_shader_secondpass->BindUniform ("cube_height");
  m_shader_secondpass->BindUniform ("cube_depth");


  m_svao->DrawElements (GL_TRIANGLES, 36, GL_UNSIGNED_INT);

  lqc::GLArrayObject::Unbind ();
  lqc::GLShader::Unbind ();
}

void GLSLRenderer::ReloadVolume (vr::Volume* volume, bool resetslicesizes)
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

  m_glsl_volume = volume->GenerateRTexture (m_init_slice_x, m_init_slice_y, m_init_slice_z, m_last_slice_x, m_last_slice_y, m_last_slice_z);
  m_shader_secondpass->SetUniformTexture3D ("volumeTexture", m_glsl_volume->GetTextureID (), 2);

  ResetUseGradient (m_glsl_use_gradient, volume);
}

void GLSLRenderer::ReloadTransferFunction (vr::TransferFunction* tfunction)
{
  if (m_glsl_transfer_function) delete m_glsl_transfer_function;
  m_glsl_transfer_function = NULL;
  
  m_glsl_transfer_function = tfunction->GenerateTexture_1D_RGBA ();

  if (m_glsl_transfer_function)
    m_shader_secondpass->SetUniformTexture1D ("transferFunctionTexture", m_glsl_transfer_function->GetTextureID (), 3);

}

void GLSLRenderer::ResetUseGradient (int use_gradient, vr::Volume* volume)
{
  if (m_glsl_gradient == NULL)
  {
    m_glsl_gradient = volume->GenerateGradientTexture (m_init_slice_x, m_init_slice_y, m_init_slice_z, m_last_slice_x, m_last_slice_y, m_last_slice_z);
    m_shader_secondpass->SetUniformTexture3D ("gradientTexture", m_glsl_gradient->GetTextureID (), 4);
  }

  m_glsl_use_gradient = use_gradient;
  m_shader_secondpass->SetUniformInt ("usegradient", m_glsl_use_gradient);
}

void GLSLRenderer::ResetSamples (int samples)
{
  if (samples >= 1)
  {
    m_glsl_samples = samples;
    m_shader_secondpass->SetUniformInt ("samples", m_glsl_samples);
  }
}

void GLSLRenderer::SetXRotation (float radius)
{
  m_x_rotation = radius;
}

float GLSLRenderer::GetXRotation ()
{
  return m_x_rotation;
}

void GLSLRenderer::SetYRotation (float radius)
{
  m_y_rotation = radius;
}

float GLSLRenderer::GetYRotation ()
{
  return m_y_rotation;
}

void GLSLRenderer::ReloadShaders ()
{
  m_shader_firstpass->Reload ();
  m_shader_secondpass->Reload ();
}

void GLSLRenderer::ResetShaders (std::string shadername)
{
  m_shader_name = shadername;

  lqc::GLShader::Unbind ();
}

int GLSLRenderer::GetInitXSlice ()
{
  return m_init_slice_x;
}

int GLSLRenderer::GetLastXSlice ()
{
  return m_last_slice_x;
}

int GLSLRenderer::GetInitYSlice ()
{
  return m_init_slice_y;
}

int GLSLRenderer::GetLastYSlice ()
{
  return m_last_slice_y;
}

int GLSLRenderer::GetInitZSlice ()
{
  return m_init_slice_z;
}

int GLSLRenderer::GetLastZSlice ()
{
  return m_last_slice_z;
}

void GLSLRenderer::SetSliceSizes (int ix, int lx, int iy, int ly, int iz, int lz)
{
  m_init_slice_x = ix;
  m_last_slice_x = lx;
  m_init_slice_y = iy;
  m_last_slice_y = ly;
  m_init_slice_z = iz;
  m_last_slice_z = lz;
}

void GLSLRenderer::SetInitXSlice (int value)
{
  m_init_slice_x = value;
}

void GLSLRenderer::SetLastXSlice (int value)
{
  m_last_slice_x = value;
}

void GLSLRenderer::SetInitYSlice (int value)
{
  m_init_slice_y = value;
}

void GLSLRenderer::SetLastYSlice (int value)
{
  m_last_slice_y = value;
}

void GLSLRenderer::SetInitZSlice (int value)
{
  m_init_slice_z = value;
}

void GLSLRenderer::SetLastZSlice (int value)
{
  m_last_slice_z = value;
}

void GLSLRenderer::AutoModeling (vr::Volume* volume)
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
}

void GLSLRenderer::ApplyModeling (float xw, float yh, float zd)
{
  m_cube_width = xw;
  m_cube_height = yh;
  m_cube_depth = zd;
}

void GLSLRenderer::SetCubeWidth (float w)
{
  m_cube_width = w;
  m_shader_secondpass->SetUniformFloat ("cube_width", m_cube_width);
}

void GLSLRenderer::SetCubeHeight (float h)
{
  m_cube_height = h;
  m_shader_secondpass->SetUniformFloat ("cube_height", m_cube_height);
}

void GLSLRenderer::SetCubeDepth (float z)
{
  m_cube_depth = z;
  m_shader_secondpass->SetUniformFloat ("cube_depth", m_cube_depth);
}

float GLSLRenderer::GetCubeWidth ()
{
  return m_cube_width;
}

float GLSLRenderer::GetCubeHeight ()
{
  return m_cube_height;
}

float GLSLRenderer::GetCubeDepth ()
{
  return m_cube_depth;
}