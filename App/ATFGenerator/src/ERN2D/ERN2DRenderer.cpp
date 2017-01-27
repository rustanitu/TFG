#include "ERN2DRenderer.h"

#include "../gbuffer.h"

#include <volrend/ScalarField.h>
#include <volrend/TransferFunction.h>
#include <glutils/GLUtils.h>
#include <math/MUtils.h>
#include <lqc/lqcdefines.h>

#include "Volume2D.h"

ERN2DRenderer::ERN2DRenderer ()
: m_FrameCount (0),
m_LastTime (0),
m_glfbo (NULL),
m_glsl_preintegrated_texture (NULL),
interaction_mode(-1)
{
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

ERN2DRenderer::~ERN2DRenderer ()
{
  Destroy ();
}

void ERN2DRenderer::CreateScene (int CurrentWidth, int CurrentHeight, vr::ScalarField* volume, gl::GLTexture* tf, bool resetslices)
{
  canvas_width = CurrentWidth;
  canvas_height = CurrentHeight;

  cam.UpdateLookAtMatrix (glm::vec3 (0, 0, 2), glm::vec3 (0, 0, 0), glm::vec3 (0, 1, 0));
  stack_matrix.MatrixMode (gl::StackMatrix::STACK::MODELVIEW);
  stack_matrix.PushIdentity ();
  stack_matrix.SetCurrentMatrix (cam.GetViewMatrix ());

  m_glfbo = new GBuffer (CurrentWidth, CurrentHeight);
  model = glm::mat4 ();

  CreateQuad ();

  Resize (CurrentWidth, CurrentHeight);

  BindShaderUniforms ();
}

bool ERN2DRenderer::Render (int Width, int Height)
{
  m_glfbo->Bind ();

  RenderQuad ();

  m_glfbo->renderBuffer (Width, Height, 0);

  GBuffer::Unbind ();
  return true;
}

void ERN2DRenderer::MouseButtonCB (int b, int p, int x, int y, char* status)
{
  if (p == 1 && b == 49)
  {
    interaction_mode = 0;
    motion_x = x;
    motion_y = y;
  }
  else if (p == 1 && b == 51)
  {
    interaction_mode = 1;
    motion_x = x;
    motion_y = y;
  }
  else
    interaction_mode = -1;
}

void ERN2DRenderer::MouseMotionCB (int x, int y, char* status)
{
  if (interaction_mode >= 0)
  {
    if (interaction_mode == 0)
    {
      int ydiff = motion_y - y;
      float x_rotation = 0.0f;
      if (ydiff != 0)
      {
        x_rotation = x_rotation + ydiff;
        motion_y = y;
      }
      lqc::RotateAroundArbitraryAxis (glm::vec3 (1, 0, 0), -x_rotation, &model);

      int xdiff = motion_x - x;
      float y_rotation = 0.0f;
      if (xdiff != 0)
      {
        y_rotation = y_rotation - xdiff;
        motion_x = x;
      }
      lqc::RotateAroundArbitraryAxis (glm::vec3 (0, 1, 0), y_rotation, &model);
    }
    else if (interaction_mode == 1)
    {
      int ydiff = motion_y - y;
      float z_rotation = 0.0f;
      if (ydiff != 0)
      {
        z_rotation = z_rotation + ydiff;
        motion_y = y;
      }
      lqc::RotateAroundArbitraryAxis (glm::vec3 (0, 0, 1), z_rotation, &model);

      int xdiff = motion_x - x;
      float y_rotation = 0.0f;
      if (xdiff != 0)
      {
        y_rotation = y_rotation - xdiff;
        motion_x = x;
      }
      lqc::RotateAroundArbitraryAxis (glm::vec3 (0, 1, 0), y_rotation, &model);
    }
  }
}

void ERN2DRenderer::Resize (int Width, int Height)
{
  canvas_width = Width;
  canvas_height = Height;
  glViewport (0, 0, Width, Height);

  stack_matrix.MatrixMode (gl::StackMatrix::STACK::PROJECTION);
  stack_matrix.LoadIdentity ();
  stack_matrix.MultiplyMatrix (cam.GenerateProjectionMatrix (45, (float)Width / (float)Height, 1.0f, 100.0f));

  m_shader_quadpass->Bind ();
  m_shader_quadpass->SetUniformMat4 ("ProjectionMatrix", stack_matrix.GetMatrix ());
  m_shader_quadpass->BindUniform ("ProjectionMatrix");
  m_shader_quadpass->Unbind ();

  m_glfbo->resize (Width, Height);
}

void ERN2DRenderer::Destroy ()
{
  DestroyQuad ();
  gl::ExitOnGLError ("ERROR: Could not destroy the shaders");

  ////////////////////////////////
  if (m_glsl_preintegrated_texture)
    delete m_glsl_preintegrated_texture;
  m_glsl_preintegrated_texture = NULL;
  ////////////////////////////////

  //////////////////
  if (m_glfbo)
    delete m_glfbo;
  m_glfbo = NULL;
  //////////////////
}

void ERN2DRenderer::BindShaderUniforms ()
{
  m_shader_quadpass->Bind ();
  m_shader_quadpass->BindUniforms ();
  m_shader_quadpass->Unbind ();
}

void ERN2DRenderer::ResetGeometry ()
{
  DestroyQuad ();
  CreateQuad ();

  BindShaderUniforms ();
}

void ERN2DRenderer::RenderQuad ()
{
  m_shader_quadpass->Bind ();
  m_shader_quadpass->SetUniformMat4 ("ModelMatrix", model);
  m_shader_quadpass->BindUniform ("ModelMatrix");

  glDisable (GL_CULL_FACE);
  GLuint attachments[1] = { GL_COLOR_ATTACHMENT0 };
  glDrawBuffers (1, attachments);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  m_qvao->DrawElements (GL_TRIANGLES, 6, GL_UNSIGNED_INT);
  
  gl::GLShader::Unbind ();
  glEnable (GL_CULL_FACE);
}

void ERN2DRenderer::CreateQuad ()
{
  float wx = m_cube_width;
  float hy = m_cube_height;

  glm::vec2 maxp = glm::vec2 (wx / 2.0f, hy / 2.0f);
  glm::vec2 minp = -maxp;

  const ERN2DRenderer::Vertex VERTICES[8] =
  {
    { { minp.x, minp.y, 0.0f }, { 0.0f, 0.0f } },
    { { minp.x, maxp.y, 0.0f }, { 0.0f, 1.0f } },
    { { maxp.x, maxp.y, 0.0f }, { 1.0f, 1.0f } },
    { { maxp.x, minp.y, 0.0f }, { 1.0f, 0.0f } },
  };
  const GLuint INDICES[6] = { 0, 2, 1, 0, 3, 2 };

  m_shader_quadpass = new gl::GLShader ("shader/StructuredDataset/ERN2D/Quad.vert",
    "shader/StructuredDataset/ERN2D/Quad.frag");

  m_shader_quadpass->SetUniformMat4 ("ModelMatrix", glm::mat4 ());
  stack_matrix.MatrixMode (gl::StackMatrix::STACK::MODELVIEW);
  m_shader_quadpass->SetUniformMat4 ("ViewMatrix", stack_matrix.GetMatrix ());
  stack_matrix.MatrixMode (gl::StackMatrix::STACK::PROJECTION);
  m_shader_quadpass->SetUniformMat4 ("ProjectionMatrix", stack_matrix.GetMatrix ());

  if (m_glsl_preintegrated_texture) m_shader_quadpass->SetUniformTexture2D ("TexturePlane", m_glsl_preintegrated_texture->GetTextureID (), 0);
  gl::ExitOnGLError ("ERROR: Could not get shader uniform locations");

  gl::GLShader::Unbind ();

  m_qvao = new gl::GLArrayObject (2);
  m_qvao->Bind ();

  m_qvbo = new gl::GLBufferObject (GL_ARRAY_BUFFER);
  m_qibo = new gl::GLBufferObject (GL_ELEMENT_ARRAY_BUFFER);

  //bind the VBO to the VAO
  m_qvbo->SetBufferData (sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);

  m_qvao->SetVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)0);
  m_qvao->SetVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)sizeof(VERTICES[0].Position));

  //bind the IBO to the VAO
  m_qibo->SetBufferData (sizeof(INDICES), INDICES, GL_STATIC_DRAW);

  gl::GLArrayObject::Unbind ();
}

void ERN2DRenderer::DestroyQuad ()
{
  if (m_qvbo)
    delete m_qvbo;
  m_qvbo = NULL;

  if (m_qibo)
    delete m_qibo;
  m_qibo = NULL;

  if (m_qvao)
    delete m_qvao;
  m_qvao = NULL;
}

void ERN2DRenderer::ReloadVolume (vr::ScalarField* volume, vr::TransferFunction* tfunction, bool resetslicesizes)
{
  if (m_glsl_preintegrated_texture) delete m_glsl_preintegrated_texture;
  m_glsl_preintegrated_texture = NULL;
  
  m_glsl_preintegrated_texture = ERNVolume2D::GenerateRGBATexture2D (volume, tfunction);
  gl::ExitOnGLError ("Error after generating RGBA Texture2D");

  if (m_glsl_preintegrated_texture) m_shader_quadpass->SetUniformTexture2D ("TexturePlane", m_glsl_preintegrated_texture->GetTextureID (), 0);
  
  m_shader_quadpass->Bind ();
  m_shader_quadpass->SetUniformTexture2D ("TexturePlane", m_glsl_preintegrated_texture->GetTextureID (), 0);
  m_shader_quadpass->BindUniform ("TexturePlane");
  gl::ExitOnGLError ("ERROR: Could not get shader uniform locations");
  gl::GLShader::Unbind ();
}

void ERN2DRenderer::SetXRotation (float radius)
{
}

float ERN2DRenderer::GetXRotation ()
{
  return -1;
}

void ERN2DRenderer::SetYRotation (float radius)
{
}

float ERN2DRenderer::GetYRotation ()
{
  return -1;
}

void ERN2DRenderer::ReloadShaders ()
{
  m_shader_quadpass->Reload ();
}

void ERN2DRenderer::ResetShaders (std::string shadername)
{
  gl::GLShader::Unbind ();
}

int ERN2DRenderer::GetInitXSlice ()
{
  return m_init_slice_x;
}

int ERN2DRenderer::GetLastXSlice ()
{
  return m_last_slice_x;
}

int ERN2DRenderer::GetInitYSlice ()
{
  return m_init_slice_y;
}

int ERN2DRenderer::GetLastYSlice ()
{
  return m_last_slice_y;
}

int ERN2DRenderer::GetInitZSlice ()
{
  return m_init_slice_z;
}

int ERN2DRenderer::GetLastZSlice ()
{
  return m_last_slice_z;
}

void ERN2DRenderer::SetSliceSizes (int ix, int lx, int iy, int ly, int iz, int lz)
{
  m_init_slice_x = ix;
  m_last_slice_x = lx;
  m_init_slice_y = iy;
  m_last_slice_y = ly;
  m_init_slice_z = iz;
  m_last_slice_z = lz;
}

void ERN2DRenderer::SetInitXSlice (int value)
{
  m_init_slice_x = value;
}

void ERN2DRenderer::SetLastXSlice (int value)
{
  m_last_slice_x = value;
}

void ERN2DRenderer::SetInitYSlice (int value)
{
  m_init_slice_y = value;
}

void ERN2DRenderer::SetLastYSlice (int value)
{
  m_last_slice_y = value;
}

void ERN2DRenderer::SetInitZSlice (int value)
{
  m_init_slice_z = value;
}

void ERN2DRenderer::SetLastZSlice (int value)
{
  m_last_slice_z = value;
}

void ERN2DRenderer::AutoModeling (vr::ScalarField* volume)
{
  if (m_glsl_preintegrated_texture)
  {
    /*int max = std::max (m_glsl_volume->GetWidth (), std::max (m_glsl_volume->GetHeight (), m_glsl_volume->GetDepth ()));
    m_cube_width = (float)m_glsl_volume->GetWidth () / (float)max;
    m_cube_height = (float)m_glsl_volume->GetHeight () / (float)max;*/
  }
  else
  {
    m_cube_width = 1.0f;
    m_cube_height = 1.0f;
  }
}

void ERN2DRenderer::ApplyModeling (float xw, float yh, float zd)
{
  m_cube_width = xw;
  m_cube_height = yh;
  m_cube_depth = zd;
}

void ERN2DRenderer::SetCubeWidth (float w)
{
  m_cube_width = w;
}

void ERN2DRenderer::SetCubeHeight (float h)
{
  m_cube_height = h;
}

void ERN2DRenderer::SetCubeDepth (float z)
{
  m_cube_depth = z;
}

float ERN2DRenderer::GetCubeWidth ()
{
  return m_cube_width;
}

float ERN2DRenderer::GetCubeHeight ()
{
  return m_cube_height;
}

float ERN2DRenderer::GetCubeDepth ()
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