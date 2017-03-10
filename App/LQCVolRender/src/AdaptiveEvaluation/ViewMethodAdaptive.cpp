#include "ViewMethodAdaptive.h"

ViewMethodAdaptive::ViewMethodAdaptive ()
  : GLSLViewMethod(VRVIEWS::ADAPTIVE_GLSL)
{
  m_volumename = "";
  m_trasnferfunctionname = "";

  m_mousepressed = false;
}

ViewMethodAdaptive::~ViewMethodAdaptive ()
{}

int ViewMethodAdaptive::Idle_Action_CB (Ihandle* cnv_renderer)
{
  IupGLMakeCurrent (cnv_renderer);
  if (m_redisplay || m_auto_redisplay)
  {
    if (m_outdated)
    {
      if (!m_renderer.m_glsl_volume || Viewer::Instance ()->m_volumename.compare (m_volumename) != 0)
      {
        m_volumename = Viewer::Instance ()->m_volumename;
        m_renderer.ReloadVolume (Viewer::Instance ()->m_volume);
      }
      if (!m_renderer.m_glsl_transfer_function || Viewer::Instance ()->m_tf_name.compare (m_trasnferfunctionname) != 0)
      {
        m_trasnferfunctionname = Viewer::Instance ()->m_tf_name;
        m_renderer.ReloadTransferFunction (Viewer::Instance ()->m_transfer_function);
      }
      UpdateIupUserInterface ();
      m_outdated = false;
    }


    if (m_renderer.m_glsl_volume && m_renderer.m_glsl_transfer_function)
    {
      m_renderer.Render (Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);
      IupGLSwapBuffers (cnv_renderer);
    }
    m_redisplay = false;
  }
  return IUP_DEFAULT;
}

int ViewMethodAdaptive::Keyboard_CB (Ihandle *ih, int c, int press)
{
  if (press == 0)
  {
    switch (c)
    {
    case K_P:
    case K_p:
      Viewer::Instance ()->SaveSnapshot ("glsl_adaptive.png");
      return IUP_DEFAULT;
      break;
    case K_R:
    case K_r:
      m_redisplay = true;
      break;
    case K_X:
    case K_x:
      break;
    case K_T:
    case K_t:
    {
      std::cout << "Set Tolerance: ";
      float tolerance;
      std::cin >> tolerance;
      m_renderer.SetRealTolerance (tolerance);
    }
    break;
    case K_M:
    case K_m:
      m_renderer.ResetModelMatrix ();
      m_redisplay = true;
      break;
    case K_V:
    case K_v:
      m_renderer.AutoResizeGeometry (Viewer::Instance ()->m_volume, Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);
      m_redisplay = true;
      break;
    case K_Z:
    case K_z:
      m_auto_redisplay = !m_auto_redisplay;
      break;
    case K_W:
    case K_w:
      break;
    case K_S:
    case K_s:
      break;
    case K_ESC:
      exit (EXIT_SUCCESS);
      break;
    default:
      break;
    }
  }
  return IUP_DEFAULT;
}

int ViewMethodAdaptive::Button_CB (Ihandle* ih, int button, int pressed, int x, int y, char* status)
{
  m_renderer.MouseButtonCb (button, pressed, x, y, status);
  m_redisplay = true;
  return IUP_DEFAULT;
}

int ViewMethodAdaptive::Motion_CB (Ihandle *ih, int x, int y, char *status)
{
  m_renderer.MouseMotionCb (x, y, status);
  m_redisplay = true;
  return IUP_DEFAULT;
}

int ViewMethodAdaptive::Resize_CB (Ihandle *ih, int width, int height)
{
  m_renderer.Resize (width, height);
  MarkOutdated ();
  return IUP_DEFAULT;
}

void ViewMethodAdaptive::CreateIupUserInterface ()
{
  m_gui.CreateIupInterface ();
}

void ViewMethodAdaptive::UpdateIupUserInterface ()
{
  m_redisplay = true;
}

void ViewMethodAdaptive::ReloadTransferFunction()
{
  m_renderer.ReloadTransferFunction(Viewer::Instance()->m_transfer_function);
  UpdateIupUserInterface();
}

void ViewMethodAdaptive::ResetCamera ()
{
  m_redisplay = true;
}

void ViewMethodAdaptive::SaveCameraState (std::string filename)
{
}

void ViewMethodAdaptive::LoadCameraState (std::string filename)
{
}

void ViewMethodAdaptive::BuildViewer ()
{
  if (!m_built)
  {
    m_renderer.CreateScene (Viewer::Instance ()->m_CurrentWidth,
      Viewer::Instance ()->m_CurrentHeight,
      Viewer::Instance ()->m_volume,
      Viewer::Instance ()->m_transfer_function->GenerateTexture_RGBA ());
    MarkOutdated ();

    printf ("GLSL - Simpson Adaptativo Built\n");
    m_built = true;
  }
}

void ViewMethodAdaptive::CleanViewer ()
{
  if (m_built)
  {
    m_renderer.Destroy ();

    printf ("GLSL - Simpson Adaptativo Cleaned\n");
    m_built = false;
  }
}

void ViewMethodAdaptive::SetUseGradient (int state)
{
}

int ViewMethodAdaptive::GetUseGradient ()
{
  return 0;
}

void ViewMethodAdaptive::SetRedisplay (bool redisp)
{
  m_redisplay = redisp;
}

void ViewMethodAdaptive::ResetShaders (std::string shadername)
{

  m_renderer.Destroy ();

  m_renderer.CreateScene (
    Viewer::Instance ()->m_CurrentWidth,
    Viewer::Instance ()->m_CurrentHeight,
    Viewer::Instance ()->m_volume,
    Viewer::Instance ()->m_transfer_function->GenerateTexture_RGBA (),
    false);
  MarkOutdated ();
}

void ViewMethodAdaptive::SetCubeWidth (float w)
{
}

void ViewMethodAdaptive::SetCubeHeight (float h)
{
}

void ViewMethodAdaptive::SetCubeDepth (float z)
{
}

float ViewMethodAdaptive::GetCubeWidth ()
{
  return 0.0f;
}

float ViewMethodAdaptive::GetCubeHeight ()
{
  return 0.0f;
}

float ViewMethodAdaptive::GetCubeDepth ()
{
  return 0.0f;
}

void ViewMethodAdaptive::SetGUICubeWidth (float w)
{
}

void ViewMethodAdaptive::SetGUICubeHeight (float h)
{
}

void ViewMethodAdaptive::SetGUICubeDepth (float z)
{
}

float ViewMethodAdaptive::GetGUICubeWidth ()
{
  return 0.0f;
}

float ViewMethodAdaptive::GetGUICubeHeight ()
{
  return 0.0f;
}

float ViewMethodAdaptive::GetGUICubeDepth ()
{
  return 0.0f;
}

void ViewMethodAdaptive::ChangeCurrentShaderProgram (int shader_id)
{
  m_renderer.shader_id = shader_id;
  CleanViewer ();
  BuildViewer ();
}

void ViewMethodAdaptive::UseDoublePrecision (bool usedoubleprec)
{
  CleanViewer ();
  BuildViewer ();
}