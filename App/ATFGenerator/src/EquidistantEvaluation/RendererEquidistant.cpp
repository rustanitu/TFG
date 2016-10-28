#include "RendererEquidistant.h"

#include <fstream>

RendererEquidistant::RendererEquidistant ()
  : m_glfbo (NULL), m_glsl_volume (NULL),
  m_glsl_transfer_function (NULL),
  shader_fpass (NULL), shader_spass (NULL),
  vao (NULL), ibo (NULL), vbo (NULL)
{
  model = glm::mat4 ();

  m_geo_cubewidth = 1.0f;
  m_geo_cubeheight = 1.0f;
  m_geo_cubedepth = 1.0f;

  interaction_mode = -1;
}

RendererEquidistant::~RendererEquidistant ()
{
  Destroy ();
}

void RendererEquidistant::Destroy ()
{
  DestroyFirstPass ();
  DestroySecondPass ();
  DestroyBuffers ();

  if (m_glsl_volume)
    delete m_glsl_volume;
  m_glsl_volume = NULL;

  if (m_glsl_transfer_function)
    delete m_glsl_transfer_function;
  m_glsl_transfer_function = NULL;

  if (m_glfbo)
    delete m_glfbo;
  m_glfbo = NULL;
}

void RendererEquidistant::CreateScene (int CurrentWidth, int CurrentHeight, vr::ScalarField* volume, gl::GLTexture1D* tf, bool resetslices)
{
  canvas_width = CurrentWidth;
  canvas_height = CurrentHeight;

  cam.UpdateLookAtMatrix (glm::vec3 (0, 0, 2), glm::vec3 (0, 0, 0), glm::vec3 (0, 1, 0));
  stack_matrix.MatrixMode (gl::StackMatrix::STACK::MODELVIEW);
  stack_matrix.PushIdentity ();
  stack_matrix.SetCurrentMatrix (cam.GetViewMatrix ());

  m_glfbo = new GBuffer (canvas_width, canvas_height);

  CreateBuffers ();
  CreateFirstPass ();
  CreateSecondPass ();

  Resize (canvas_width, canvas_height);
  
  shader_fpass->Bind ();
  shader_fpass->BindUniforms ();
  shader_fpass->Unbind ();

  shader_spass->Bind ();
  shader_spass->BindUniforms ();
  shader_spass->Unbind ();
}

bool RendererEquidistant::Render (int Width, int Height)
{
  m_glfbo->Bind ();

  DrawFirstPass ();
  DrawSecondPass ();

  m_glfbo->renderBuffer (Width, Height, 0);

  GBuffer::Unbind ();
  return true;
}

void RendererEquidistant::Resize (int Width, int Height)
{
  canvas_width = Width;
  canvas_height = Height;
  glViewport (0, 0, Width, Height);

  stack_matrix.MatrixMode (gl::StackMatrix::STACK::PROJECTION);
  stack_matrix.LoadIdentity ();
  stack_matrix.MultiplyMatrix (cam.GenerateProjectionMatrix (45, (float)Width / (float)Height, 1.0f, 100.0f));

  shader_fpass->Bind ();
  shader_fpass->SetUniformMat4 ("ProjectionMatrix", stack_matrix.GetMatrix ());
  shader_fpass->BindUniform ("ProjectionMatrix");
  shader_fpass->Unbind ();
  
  shader_spass->Bind ();
  shader_spass->SetUniformMat4 ("ProjectionMatrix", stack_matrix.GetMatrix ());
  shader_spass->BindUniform ("ProjectionMatrix");
  shader_spass->SetUniformInt ("ScreenSizeW", canvas_width);
  shader_spass->BindUniform ("ScreenSizeW");
  shader_spass->SetUniformInt ("ScreenSizeH", canvas_height);
  shader_spass->BindUniform ("ScreenSizeH");
  shader_spass->Unbind ();

  m_glfbo->resize (Width, Height);
}

void RendererEquidistant::MouseButtonCb (int b, int p, int x, int y, char* st)
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

void RendererEquidistant::MouseMotionCb (int x, int y, char *status)
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
    }
  }
}

void RendererEquidistant::ReloadVolume (vr::ScalarField* volume)
{
  if (m_glsl_volume) delete m_glsl_volume;
  m_glsl_volume = NULL;

  m_glsl_volume = vr::GenerateRTexture (volume, 0, 0, 0, volume->GetWidth (), volume->GetHeight (), volume->GetDepth ());
  if (m_glsl_volume)
  {
    shader_spass->Bind ();
    shader_spass->SetUniformTexture3D ("txt3D_volume", m_glsl_volume->GetTextureID (), 1);
    shader_spass->BindUniform ("txt3D_volume");

    shader_spass->SetUniformInt ("VolWidth", m_glsl_volume->GetWidth ());
    shader_spass->SetUniformInt ("VolHeight", m_glsl_volume->GetHeight ());
    shader_spass->SetUniformInt ("VolDepth", m_glsl_volume->GetDepth ());
    shader_spass->BindUniform ("VolWidth");
    shader_spass->BindUniform ("VolHeight");
    shader_spass->BindUniform ("VolDepth");

    shader_spass->SetUniformVector3f ("tex_scale", lqc::Vector3f (
      (1.0f / m_geo_cubewidth) / (float)m_glsl_volume->GetWidth (),
      (1.0f / m_geo_cubeheight) / (float)m_glsl_volume->GetHeight (),
      (1.0f / m_geo_cubedepth) / (float)m_glsl_volume->GetDepth ()
      ));
    shader_spass->BindUniform ("tex_scale");

    shader_spass->Unbind ();

    AutoResizeGeometry (volume, canvas_width, canvas_height);
  }
}

void RendererEquidistant::ReloadTransferFunction (vr::TransferFunction* tfunction)
{
  if (m_glsl_transfer_function) delete m_glsl_transfer_function;
  m_glsl_transfer_function = NULL;

  m_glsl_transfer_function = tfunction->GenerateTexture_1D_RGBA ();

  if (m_glsl_transfer_function)
  {
    shader_spass->Bind ();
    shader_spass->SetUniformTexture1D ("txt1D_transferfunction", m_glsl_transfer_function->GetTextureID (), 2);
    shader_spass->BindUniform ("txt1D_transferfunction");
    shader_spass->Unbind ();
  }
}

void RendererEquidistant::ResetModelMatrix ()
{
  model = glm::mat4 ();
}

void RendererEquidistant::AutoResizeGeometry (vr::ScalarField* volume, int Width, int Height, bool stop_bef_shad_operations)
{
  if (m_glsl_volume)
  {
    int max = std::max (m_glsl_volume->GetWidth (), std::max (m_glsl_volume->GetHeight (), m_glsl_volume->GetDepth ()));
    m_geo_cubewidth = (float)m_glsl_volume->GetWidth () / (float)max;
    m_geo_cubeheight = (float)m_glsl_volume->GetHeight () / (float)max;
    m_geo_cubedepth = (float)m_glsl_volume->GetDepth () / (float)max;
  }
  else
  {
    m_geo_cubewidth = 1.0f;
    m_geo_cubeheight = 1.0f;
    m_geo_cubedepth = 1.0f;
  }

  if (stop_bef_shad_operations) return;

  shader_spass->Bind ();
  shader_spass->SetUniformVector3f ("tex_scale", lqc::Vector3f (
    (1.0f / m_geo_cubewidth) / (float)m_glsl_volume->GetWidth (),
    (1.0f / m_geo_cubeheight) / (float)m_glsl_volume->GetHeight (),
    (1.0f / m_geo_cubedepth) / (float)m_glsl_volume->GetDepth ()));
  shader_spass->BindUniform ("tex_scale");
  shader_spass->Unbind ();

  //Reset Geometry
  DestroyBuffers ();
  DestroyFirstPass ();
  DestroySecondPass ();

  CreateBuffers ();
  CreateFirstPass ();
  CreateSecondPass ();

  shader_fpass->Bind ();
  shader_fpass->BindUniforms ();
  shader_fpass->Unbind ();

  shader_spass->Bind ();
  shader_spass->BindUniforms ();
  shader_spass->Unbind ();

  //Resize screen
  Resize (Width, Height);
}

void RendererEquidistant::SetStepDistance (float stp)
{
  shader_spass->Bind ();
  shader_spass->SetUniformFloat ("step_size", stp);
  shader_spass->BindUniform ("step_size");
  shader_spass->Unbind ();
}

void RendererEquidistant::DrawFirstPass ()
{
  shader_fpass->Bind ();
  shader_fpass->SetUniformMat4 ("ModelMatrix", model);
  stack_matrix.MatrixMode (gl::StackMatrix::STACK::MODELVIEW);
  shader_fpass->SetUniformMat4 ("ViewMatrix", stack_matrix.GetMatrix ());

  shader_fpass->BindUniform ("ModelMatrix");
  shader_fpass->BindUniform ("ViewMatrix");

  gl::GLShader::Unbind();

  {
    glCullFace (GL_FRONT);
    GLuint attachments[1] = { GL_COLOR_ATTACHMENT1 };
    glDrawBuffers (1, attachments);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader_fpass->Bind();

    vao->DrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT);
  }

  gl::GLArrayObject::Unbind ();
  gl::GLShader::Unbind ();

  glCullFace(GL_BACK);
}

void RendererEquidistant::DrawSecondPass ()
{
  shader_spass->Bind ();
  shader_spass->SetUniformMat4 ("ModelMatrix", model);
  stack_matrix.MatrixMode (gl::StackMatrix::STACK::MODELVIEW);
  shader_spass->SetUniformMat4 ("ViewMatrix", stack_matrix.GetMatrix ());

  shader_spass->BindUniform ("ModelMatrix");
  shader_spass->BindUniform ("ViewMatrix");
  shader_spass->BindUniform ("txt2D_exitpoints");

  gl::GLShader::Unbind();

  glCullFace(GL_BACK);
  GLuint attachments[1] = { GL_COLOR_ATTACHMENT0 };
  glDrawBuffers (1, attachments);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  shader_spass->Bind();

  svao->DrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT);

  gl::GLArrayObject::Unbind ();
  gl::GLShader::Unbind ();
}

void RendererEquidistant::CreateFirstPass ()
{
  shader_fpass = new gl::GLShader ("shader/StructuredDataset/backface.vert",
    "shader/StructuredDataset/backface.frag");

  shader_fpass->SetUniformMat4 ("ModelMatrix", glm::mat4 ());
  stack_matrix.MatrixMode (gl::StackMatrix::STACK::MODELVIEW);
  shader_fpass->SetUniformMat4 ("ViewMatrix", stack_matrix.GetMatrix ());
  stack_matrix.MatrixMode (gl::StackMatrix::STACK::PROJECTION);
  shader_fpass->SetUniformMat4 ("ProjectionMatrix", stack_matrix.GetMatrix ());

#ifdef LQC_GL_UTILS_LIB
#ifdef _DEBUG 
  gl::ExitOnGLError ("ERROR: Could not get shader uniform locations");
#endif
#endif

  gl::GLShader::Unbind ();
}

void RendererEquidistant::CreateSecondPass ()
{
  shader_spass = new gl::GLShader ("shader/StructuredDataset/raycasting.vert",
    "shader/StructuredDataset/EquidistantEvaluation/riemann.frag");

  shader_spass->SetUniformMat4 ("ModelMatrix", glm::mat4 ());
  stack_matrix.MatrixMode (gl::StackMatrix::STACK::MODELVIEW);
  shader_spass->SetUniformMat4 ("ViewMatrix", stack_matrix.GetMatrix ());
  stack_matrix.MatrixMode (gl::StackMatrix::STACK::PROJECTION);
  shader_spass->SetUniformMat4 ("ProjectionMatrix", stack_matrix.GetMatrix ());

  shader_spass->SetUniformTexture2D ("txt2D_exitpoints", m_glfbo->m_backtexture, 0);
  if (m_glsl_volume) shader_spass->SetUniformTexture3D ("txt3D_volume", m_glsl_volume->GetTextureID (), 1);
  if (m_glsl_transfer_function) shader_spass->SetUniformTexture1D ("txt1D_transferfunction", m_glsl_transfer_function->GetTextureID (), 2);

  shader_spass->SetUniformInt ("ScreenSizeW", canvas_width);
  shader_spass->SetUniformInt ("ScreenSizeH", canvas_height);

  if (m_glsl_volume)
  {
    shader_spass->SetUniformInt ("VolWidth", m_glsl_volume->GetWidth ());
    shader_spass->SetUniformInt ("VolHeight", m_glsl_volume->GetHeight ());
    shader_spass->SetUniformInt ("VolDepth", m_glsl_volume->GetDepth ());
  }
  else
  {
    shader_spass->SetUniformInt ("VolWidth", 1);
    shader_spass->SetUniformInt ("VolHeight", 1);
    shader_spass->SetUniformInt ("VolDepth", 1);
  }

  float geomWidth = 1.0f / m_geo_cubewidth;
  float geomHeight = 1.0f / m_geo_cubeheight;
  float geomDepth = 1.0f / m_geo_cubedepth;

  float volwidth = 1.0f;
  float volheight = 1.0f;
  float voldepth = 1.0f;
  if (m_glsl_volume)
  {
    volwidth = (float)m_glsl_volume->GetWidth ();
    volheight = (float)m_glsl_volume->GetHeight ();
    voldepth = (float)m_glsl_volume->GetDepth ();
  }

  shader_spass->SetUniformVector3f ("tex_scale", lqc::Vector3f (
    geomWidth / volwidth, geomHeight / volheight, geomDepth / voldepth));

  shader_spass->SetUniformFloat ("step_size", .5f);

#ifdef LQC_GL_UTILS_LIB
#ifdef _DEBUG 
  gl::ExitOnGLError ("ERROR: Could not get shader uniform locations");
#endif
#endif

  gl::GLShader::Unbind ();
}

void RendererEquidistant::CreateBuffers ()
{
  float wx = m_geo_cubewidth;
  float hy = m_geo_cubeheight;
  float dz = m_geo_cubedepth;

  glm::vec3 maxp = glm::vec3 (wx / 2.0f, hy / 2.0f, dz / 2.0f);
  glm::vec3 minp = -maxp;

  const RendererEquidistant::Vertex VERTICES[8] =
  {
    { { minp.x, minp.y, maxp.z }, { 0.0f, 0.0f, dz } },
    { { minp.x, maxp.y, maxp.z }, { 0.0f, hy, dz } },
    { { maxp.x, maxp.y, maxp.z }, { wx, hy, dz } },
    { { maxp.x, minp.y, maxp.z }, { wx, 0.0f, dz } },
    { { minp.x, minp.y, minp.z }, { 0.0f, 0.0f, 0.0f } },
    { { minp.x, maxp.y, minp.z }, { 0.0f, hy, 0.0f } },
    { { maxp.x, maxp.y, minp.z }, { wx, hy, 0.0f } },
    { { maxp.x, minp.y, minp.z }, { wx, 0.0f, 0.0f } }
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

  vao = new gl::GLArrayObject (2);
  vao->Bind ();

  vbo = new gl::GLBufferObject (GL_ARRAY_BUFFER);
  ibo = new gl::GLBufferObject (GL_ELEMENT_ARRAY_BUFFER);

  //bind the VBO to the VAO
  vbo->SetBufferData (sizeof (VERTICES), VERTICES, GL_STATIC_DRAW);

  vao->SetVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, sizeof (VERTICES[0]), (GLvoid*)0);
  vao->SetVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, sizeof (VERTICES[0]), (GLvoid*)sizeof (VERTICES[0].Position));

  //bind the IBO to the VAO
  ibo->SetBufferData (sizeof (INDICES), INDICES, GL_STATIC_DRAW);

  gl::GLArrayObject::Unbind ();

  svao = new gl::GLArrayObject(2);
  svao->Bind();

  svbo = new gl::GLBufferObject(GL_ARRAY_BUFFER);
  sibo = new gl::GLBufferObject(GL_ELEMENT_ARRAY_BUFFER);

  //bind the VBO to the VAO
  svbo->SetBufferData(sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);

  svao->SetVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)0);
  svao->SetVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)sizeof(VERTICES[0].Position));

  //bind the IBO to the VAO
  sibo->SetBufferData(sizeof(INDICES), INDICES, GL_STATIC_DRAW);

  gl::GLArrayObject::Unbind();
}

void RendererEquidistant::DestroyFirstPass ()
{
  if (shader_fpass)
    delete shader_fpass;
  shader_fpass = NULL;

#ifdef LQC_GL_UTILS_LIB
#ifdef _DEBUG 
  gl::ExitOnGLError ("ERROR: Could not destroy the shaders");
#endif
#endif
}

void RendererEquidistant::DestroySecondPass ()
{
  if (shader_spass)
    delete shader_spass;
  shader_spass = NULL;

#ifdef LQC_GL_UTILS_LIB
#ifdef _DEBUG 
  gl::ExitOnGLError ("ERROR: Could not destroy the shaders");
#endif
#endif
}

void RendererEquidistant::DestroyBuffers ()
{
  if (vbo)
    delete vbo;
  vbo = NULL;

  if (ibo)
    delete ibo;
  ibo = NULL;

  if (vao)
    delete vao;
  vao = NULL;

  if (svbo)
    delete svbo;
  svbo = NULL;

  if (sibo)
    delete sibo;
  sibo = NULL;

  if (svao)
    delete svao;
  svao = NULL;
}
