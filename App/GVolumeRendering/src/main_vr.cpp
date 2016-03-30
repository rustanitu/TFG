#include <volrend/Volume.h>
#include <volrend/TransferFunction.h>
#include <volrend/Reader.h>
#include <volrend/Utils.h>

#include <math/Matrix4.h>
#include <math/MUtils.h>

#include <glutils/GLShader.h>
#include <glutils/GLCamera.h>
#include <glutils/GLArrayObject.h>
#include <glutils/GLBufferObject.h>

#include <lqc/image/image.h>

#include "gbuffer.h"

#include <cstdio>

#include <GL/glew.h>
#include <GL/freeglut.h>

typedef struct Vertex
{
  float Position[3];
  float Color[3];
} Vertex;

int shader_width  = 800;
int shader_height = 600;
int WindowHandle = 0;
int FrameCount = 0, LastTime = 0;

float x_rotation = 0.0f;
float y_rotation = 0.0f;

float initial_step = 0.5f;
float tol_error = .1f;

lqc::Matrix4f
ProjectionMatrix,
ViewMatrix,
ModelMatrix;

GBuffer *glfbo                            = NULL;
vr::Volume* vr_volume                     = NULL;
vr::TransferFunction* vr_trasnferfunction = NULL;
gl::GLTexture3D* glsl_volume             = NULL;
gl::GLTexture1D* glsl_transfer_function  = NULL;
gl::GLShader* shader_firstpass           = NULL;
gl::GLShader* shader_secondpass          = NULL;
gl::GLArrayObject*  fvao                 = NULL;
gl::GLBufferObject* fvbo                 = NULL;
gl::GLBufferObject* fibo                 = NULL;
gl::GLArrayObject*  svao                 = NULL;
gl::GLBufferObject* svbo                 = NULL;
gl::GLBufferObject* sibo                 = NULL;

void Initialize (int, char*[]);
void InitWindow (int, char*[]);
void CreateScene ();
void BindShaderUniforms ();
void Destroy ();
bool Render (int Width, int Height);
void Resize (int Width, int Height);
void Render (void);
void MouseFunction (int button, int state, int x, int y);
void MouseMotionFunction (int x, int y);
void KeyboardDownFunction (unsigned char c, int x, int y);

void CreateFirstPass ();
void CreateSecondPass ();

void FirstPass ();
void SecondPass ();

bool mousepressed;
int motion_x, motion_y;

float cube_width = 1.0f, cube_height = 1.0f, cube_depth = 1.0f;

int main (int argc, char* argv[])
{
  int* f;
  f = new int[16];
  for (int i = 0; i < 16; i++)
    f[i] = 1;

  int* sat = lqc::SummedAreaTable<int> (f, 4, 4);
  
  for (int j = 3; j >= 0; j--)
  {
    for (int i = 0; i < 4; i++)
    {
      printf ("%d ", sat[i + (j * 4)]);
    }
    printf ("\n");
  }

  Initialize (argc, argv);
  CreateScene ();

  glutMainLoop ();

  Destroy ();

  exit (EXIT_SUCCESS);
  return 0;
}

void Initialize (int argc, char* argv[])
{
  GLenum GlewInitResult;

  InitWindow (argc, argv);

  glewExperimental = GL_TRUE;
  GlewInitResult = glewInit ();

  if (GLEW_OK != GlewInitResult)
  {
    fprintf (stderr, "ERROR: %s\n", glewGetErrorString (GlewInitResult));
    exit (EXIT_FAILURE);
  }

  fprintf (stdout, "INFO: OpenGL Version: %s\n", glGetString (GL_VERSION));

  glGetError ();
  glClearColor (1.0f, 1.0f, 1.0f, 1.0f);

  glEnable (GL_DEPTH_TEST);
  glDepthFunc (GL_LESS);
  gl::ExitOnGLError ("ERROR: Could not set OpenGL depth testing options");

  glEnable (GL_CULL_FACE);
  gl::ExitOnGLError ("ERROR: Could not set OpenGL culling options");
  
  glEnable (GL_BLEND);

  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  gl::ExitOnGLError ("ERROR: OpenGL error");
}
 
void CreateScene ()
{
  shader_width = 800;
  shader_height = 600;

  ModelMatrix = lqc::IDENTITY_MATRIX;
  ProjectionMatrix = lqc::IDENTITY_MATRIX;
  ViewMatrix = lqc::IDENTITY_MATRIX;
  float zcam = -2.0f;
  lqc::TranslateMatrix (&ViewMatrix, 0, 0, zcam);

  vr_volume = vr::ReadFromVolMod ("../../resources/VolumeModels/Bonsai.1.256x256x256.raw"); 
  vr_trasnferfunction = vr::ReadTransferFunction ("../../resources/TransferFunctions/Bonsai.1.256x256x256.linear.tf1d");

  glsl_volume = vr::GenerateRTexture (vr_volume, 0, 0, 0
    , vr_volume->GetWidth (), vr_volume->GetHeight ()
    , vr_volume->GetDepth ());
  glsl_transfer_function = vr_trasnferfunction->GenerateTexture_1D_RGBA ();

  glfbo = new GBuffer (shader_width, shader_height);

  CreateFirstPass ();
  CreateSecondPass ();

  Resize (shader_width, shader_height);

  BindShaderUniforms ();
}

void Render (void)
{
  ++FrameCount;

  clock_t Now = clock ();

  if (LastTime == 0)
    LastTime = Now;

  ModelMatrix = lqc::IDENTITY_MATRIX;
  lqc::RotateAboutX (&ModelMatrix, x_rotation * (float)PI / 180.0f);
  lqc::RotateAboutY (&ModelMatrix, y_rotation * (float)PI / 180.0f);
  //lqc::TranslateMatrix (&m_ModelMatrix, 0, 0, 0.2);

  gl::GLShader::Unbind ();

  glfbo->Bind ();
  FirstPass ();
  SecondPass ();

  glfbo->renderBuffer (shader_width, shader_height, 0);
  //glfbo->renderBuffers (shader_width, shader_height);

  GBuffer::Unbind ();
  LastTime = Now;
  glutSwapBuffers ();
  glutPostRedisplay ();
}

void InitWindow (int argc, char* argv[])
{
  glutInit (&argc, argv);

  glutInitContextVersion (4, 3);
  glutInitContextFlags (GLUT_FORWARD_COMPATIBLE);
  glutInitContextProfile (GLUT_CORE_PROFILE);

  glutSetOption (
    GLUT_ACTION_ON_WINDOW_CLOSE,
    GLUT_ACTION_GLUTMAINLOOP_RETURNS
    );

  glutInitWindowSize (shader_width, shader_height);

  glutInitDisplayMode (GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);

  WindowHandle = glutCreateWindow ("Volume Rendering");

  if (WindowHandle < 1) {
    fprintf (
      stderr,
      "ERROR: Could not create a new rendering window.\n"
      );
    exit (EXIT_FAILURE);
  }

  glutReshapeFunc (Resize);
  glutDisplayFunc (Render);
  //glutIdleFunc (IdleFunction);
  glutKeyboardFunc (KeyboardDownFunction);
  glutMouseFunc (MouseFunction);
  glutMotionFunc (MouseMotionFunction);
  glutCloseFunc (Destroy);
}

void MouseFunction (int button, int state, int x, int y)
{
  if (state == 0)
  {
    mousepressed = true;
    motion_x = x;
    motion_y = y;
  }
  else
    mousepressed = false;
}

void MouseMotionFunction (int x, int y)
{
  if (mousepressed)
  {
    int ydiff = motion_y - y;
    if (ydiff != 0)
    {
      x_rotation = x_rotation + ydiff;
      motion_y = y;
    }
    int xdiff = motion_x - x;
    if (xdiff != 0)
    {
      y_rotation = y_rotation - xdiff;
      motion_x = x;
    }
  }
}

void CreateFirstPass ()
{
  float wx = cube_width;
  float hy = cube_height;
  float dz = cube_depth;

  glm::vec3 maxp = glm::vec3 (wx / 2.0f, hy / 2.0f, dz / 2.0f);
  glm::vec3 minp = -maxp;

  const Vertex VERTICES[8] =
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


  shader_firstpass = new gl::GLShader (
    "shader/StructuredDataset/backface.vert",
    "shader/StructuredDataset/backface.frag"
  );

  shader_firstpass->Bind();
  shader_firstpass->SetUniformMatrix4f ("ModelMatrix", lqc::IDENTITY_MATRIX);
  shader_firstpass->SetUniformMatrix4f ("ViewMatrix", lqc::IDENTITY_MATRIX);
  shader_firstpass->SetUniformMatrix4f ("ProjectionMatrix", lqc::IDENTITY_MATRIX);
  
  gl::ExitOnGLError ("ERROR: Could not get shader uniform locations");

  gl::GLShader::Unbind ();

  fvao = new gl::GLArrayObject (2);
  fvao->Bind ();

  fvbo = new gl::GLBufferObject (gl::GLBufferObject::TYPES::VERTEXBUFFEROBJECT);
  fibo = new gl::GLBufferObject (gl::GLBufferObject::TYPES::INDEXBUFFEROBJECT);

  //bind the VBO to the VAO
  fvbo->SetBufferData (sizeof (VERTICES), VERTICES, GL_STATIC_DRAW);

  fvao->SetVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, sizeof (VERTICES[0]), (GLvoid*)0);
  fvao->SetVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, sizeof (VERTICES[0]), (GLvoid*)sizeof (VERTICES[0].Position));

  //bind the IBO to the VAO
  fibo->SetBufferData (sizeof (INDICES), INDICES, GL_STATIC_DRAW);

  gl::GLArrayObject::Unbind ();
}

void CreateSecondPass ()
{
  float wx = cube_width;
  float hy = cube_height;
  float dz = cube_depth;

  glm::vec3 maxp = glm::vec3 (wx / 2.0f, hy / 2.0f, dz / 2.0f);
  glm::vec3 minp = -maxp;

  const Vertex VERTICES[8] =
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


  shader_secondpass = new gl::GLShader (
    "shader/StructuredDataset/raycasting.vert",
    "shader/StructuredDataset/RiemannSummation/raycasting.frag"
  );

  shader_secondpass->Bind ();
  shader_secondpass->SetUniformMatrix4f ("ModelMatrix", lqc::IDENTITY_MATRIX);
  shader_secondpass->SetUniformMatrix4f ("ViewMatrix", lqc::IDENTITY_MATRIX);
  shader_secondpass->SetUniformMatrix4f ("ProjectionMatrix", lqc::IDENTITY_MATRIX);

  shader_secondpass->SetUniformTexture2D ("ExitPoints", glfbo->m_backtexture, 0);
  if (glsl_volume) shader_secondpass->SetUniformTexture3D ("VolumeTex", glsl_volume->GetTextureID (), 1);
  if (glsl_transfer_function) shader_secondpass->SetUniformTexture1D ("TransferFunc", glsl_transfer_function->GetTextureID (), 2);

  shader_secondpass->SetUniformInt ("ScreenSizeW", shader_width);
  shader_secondpass->SetUniformInt ("ScreenSizeH", shader_height);

  shader_secondpass->SetUniformInt ("VolWidth", glsl_volume->GetWidth ());
  shader_secondpass->SetUniformInt ("VolHeight", glsl_volume->GetHeight ());
  shader_secondpass->SetUniformInt ("VolDepth", glsl_volume->GetDepth ());

  shader_secondpass->SetUniformVector3f ("tex_scale", lqc::Vector3f (
    (1.0f / cube_width) / (float)glsl_volume->GetWidth (),
    (1.0f / cube_height) / (float)glsl_volume->GetHeight (),
    (1.0f / cube_depth) / (float)glsl_volume->GetDepth ()
  ));
  
  shader_secondpass->SetUniformFloat("step_size", 0.5f);

  gl::ExitOnGLError ("ERROR: Could not get shader uniform locations");

  gl::GLShader::Unbind ();

  svao = new gl::GLArrayObject (2);
  svao->Bind ();

  svbo = new gl::GLBufferObject (GL_ARRAY_BUFFER);
  sibo = new gl::GLBufferObject (GL_ELEMENT_ARRAY_BUFFER);

  //bind the VBO to the VAO
  svbo->SetBufferData (sizeof (VERTICES), VERTICES, GL_STATIC_DRAW);

  svao->SetVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, sizeof (VERTICES[0]), (GLvoid*)0);
  svao->SetVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, sizeof (VERTICES[0]), (GLvoid*)sizeof (VERTICES[0].Position));

  //bind the IBO to the VAO
  sibo->SetBufferData (sizeof (INDICES), INDICES, GL_STATIC_DRAW);

  gl::GLArrayObject::Unbind ();
}

void FirstPass ()
{
  shader_firstpass->Bind ();
  shader_firstpass->SetUniformMatrix4f ("ModelMatrix", ModelMatrix);
  shader_firstpass->SetUniformMatrix4f ("ViewMatrix", ViewMatrix);
  shader_firstpass->BindUniform ("ModelMatrix");
  shader_firstpass->BindUniform ("ViewMatrix");

  {
    glCullFace (GL_FRONT);
    GLuint attachments[1] = { GL_COLOR_ATTACHMENT1 };
    glDrawBuffers (1, attachments);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    fvao->DrawElements (GL_TRIANGLES, 36, GL_UNSIGNED_INT);
  }

  glCullFace (GL_BACK);
}

void SecondPass ()
{
  shader_secondpass->Bind ();
  shader_secondpass->SetUniformMatrix4f ("ModelMatrix", ModelMatrix);
  shader_secondpass->SetUniformMatrix4f ("ViewMatrix",  ViewMatrix);
  shader_secondpass->SetUniformTexture2D ("ExitPoints", glfbo->m_backtexture, 0);
  shader_secondpass->BindUniform ("ModelMatrix");
  shader_secondpass->BindUniform ("ViewMatrix");
  shader_secondpass->BindUniform ("ExitPoints");

  glCullFace (GL_BACK);
  GLuint attachments[1] = { GL_COLOR_ATTACHMENT0 };
  glDrawBuffers (1, attachments);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  svao->DrawElements (GL_TRIANGLES, 36, GL_UNSIGNED_INT);

  gl::GLArrayObject::Unbind ();
  gl::GLShader::Unbind ();
}

void BindShaderUniforms ()
{
  shader_firstpass->Bind ();
  shader_firstpass->BindUniforms ();
  shader_firstpass->Unbind ();

  shader_secondpass->Bind ();
  shader_secondpass->BindUniforms ();
  shader_secondpass->Unbind ();
}

void DestroyCube ()
{
  if (fvbo)
    delete fvbo;
  fvbo = NULL;

  if (fibo)
    delete fibo;
  fibo = NULL;

  if (fvao)
    delete fvao;
  fvao = NULL;

  if (shader_firstpass)
    delete shader_firstpass;
  shader_firstpass = NULL;
}

void DestroyQuad ()
{
  if (svbo)
    delete svbo;
  svbo = NULL;

  if (sibo)
    delete sibo;
  sibo = NULL;

  if (svao)
    delete svao;
  svao = NULL;

  if (shader_secondpass)
    delete shader_secondpass;
  shader_secondpass = NULL;
}

void Destroy ()
{
  DestroyCube ();
  DestroyQuad ();
  gl::ExitOnGLError ("ERROR: Could not destroy the shaders");

  ////////////////////////////////
  if (glsl_volume)
    delete glsl_volume;
  glsl_volume = NULL;

  if (glsl_transfer_function)
    delete glsl_transfer_function;
  glsl_transfer_function = NULL;
  ////////////////////////////////

  //////////////////
  if (glfbo)
    delete glfbo;
  glfbo = NULL;
  //////////////////
}

void Resize (int Width, int Height)
{
  shader_width = Width;
  shader_height = Height;
  glViewport (0, 0, Width, Height);
  ProjectionMatrix = lqc::CreateProjectionMatrix (45, (float)Width / (float)Height, 1.0f, 100.0f);

  shader_firstpass->Bind ();
  shader_firstpass->SetUniformMatrix4f ("ProjectionMatrix", ProjectionMatrix);
  shader_firstpass->BindUniform ("ProjectionMatrix");
  shader_firstpass->Unbind ();

  shader_secondpass->Bind ();
  shader_secondpass->SetUniformMatrix4f ("ProjectionMatrix", ProjectionMatrix);
  shader_secondpass->BindUniform ("ProjectionMatrix");
  shader_secondpass->SetUniformInt ("ScreenSizeW", shader_width);
  shader_secondpass->BindUniform ("ScreenSizeW");
  shader_secondpass->SetUniformInt ("ScreenSizeH", shader_height);
  shader_secondpass->BindUniform ("ScreenSizeH");
  shader_secondpass->Unbind ();

  glfbo->resize (Width, Height);
}

void SetRealTolerance (float tol)
{
  tol_error = tol;

  shader_secondpass->Bind ();
  shader_secondpass->SetUniformFloat ("tol_error", tol);

  shader_secondpass->BindUniform ("tol_error");
  shader_secondpass->Unbind ();
}

void KeyboardDownFunction (unsigned char c, int x, int y)
{
  switch (c)
  {
    case 'T':
    case 't':
      {
        std::cout << "Set Tolerance: ";
        float tolerance;
        std::cin >> tolerance;
        SetRealTolerance (tolerance);
      }
      break;
    case 'M':
    case 'm':
      x_rotation = 0.0f;
      y_rotation = 0.0f;
      break;
    case 'V':
    case 'v':
      Destroy ();
      CreateScene ();
      break;
    case 27:
      exit (EXIT_SUCCESS);
      break;
    default:
      break;
  }
}