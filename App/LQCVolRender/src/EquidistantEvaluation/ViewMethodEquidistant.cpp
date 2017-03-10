#include "ViewMethodEquidistant.h"

ViewMethodEquidistant::ViewMethodEquidistant ()
  : GLSLViewMethod(VRVIEWS::EQUIDISTANT_GLSL)
{
  m_volumename = "";
  m_trasnferfunctionname = "";

  m_mousepressed = false;
}

ViewMethodEquidistant::~ViewMethodEquidistant ()
{}

char* ViewMethodEquidistant::GetViewMethodName ()
{
  return "ViewMethodEquidistant";
}

int ViewMethodEquidistant::Idle_Action_CB (Ihandle* cnv_renderer)
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

int ViewMethodEquidistant::Keyboard_CB (Ihandle *ih, int c, int press)
{
  if (press == 0)
  {
    switch (c)
    {
      case K_P:
      case K_p:
        Viewer::Instance ()->SaveSnapshot ("glsl_equidistant.png");
        return IUP_DEFAULT;
      break;
      case K_R:
      case K_r:
        m_redisplay = true;
        break;
      case K_X:
      case K_x:
        break;
      case K_D:
      case K_d:
        {
          std::cout << "Set Step Distance: ";
          float step;
          std::cin >> step;
          m_renderer.SetStepDistance (step);
        }
        break;
      case K_M:
      case K_m:
        m_renderer.ResetModelMatrix ();
        m_redisplay = true;
        break;
        break;
      case K_Z:
      case K_z:
        m_auto_redisplay = !m_auto_redisplay;
        break;
      case K_V:
      case K_v:
        m_renderer.AutoResizeGeometry (Viewer::Instance ()->m_volume, Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);
        m_redisplay = true;
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

int ViewMethodEquidistant::Button_CB (Ihandle* ih, int button, int pressed, int x, int y, char* status)
{
  m_renderer.MouseButtonCb (button, pressed, x, y, status);
  m_redisplay = true;
  return IUP_DEFAULT;
}

int ViewMethodEquidistant::Motion_CB (Ihandle *ih, int x, int y, char *status)
{
  m_renderer.MouseMotionCb (x, y, status);
  m_redisplay = true;
  return IUP_DEFAULT;
}

int ViewMethodEquidistant::Resize_CB (Ihandle *ih, int width, int height)
{
  m_renderer.Resize (width, height);
  MarkOutdated ();
  return IUP_DEFAULT;
}

void ViewMethodEquidistant::CreateIupUserInterface ()
{
  m_gui.CreateIupInterface ();
}

void ViewMethodEquidistant::UpdateIupUserInterface ()
{
}

void ViewMethodEquidistant::ReloadTransferFunction()
{
  m_renderer.ReloadTransferFunction(Viewer::Instance()->m_transfer_function);
  UpdateIupUserInterface();
}

void ViewMethodEquidistant::ResetCamera ()
{
  m_redisplay = true;
}

void ViewMethodEquidistant::SaveCameraState (std::string filename)
{
}

void ViewMethodEquidistant::LoadCameraState (std::string filename)
{
}

void ViewMethodEquidistant::BuildViewer ()
{
  if (!m_built)
  {
    m_renderer.CreateScene (Viewer::Instance ()->m_CurrentWidth,
      Viewer::Instance ()->m_CurrentHeight,
      Viewer::Instance ()->m_volume,
      Viewer::Instance ()->m_transfer_function->GenerateTexture_RGBA ());
    MarkOutdated ();
    
    printf ("GLSL - Equidistante Built\n");
    m_built = true;
  }
}

void ViewMethodEquidistant::CleanViewer ()
{
  if (m_built)
  {
    m_renderer.Destroy ();

    printf ("GLSL - Equidistante Cleaned\n");
    m_built = false;
  }
}

void ViewMethodEquidistant::SetUseGradient (int state)
{
}

int ViewMethodEquidistant::GetUseGradient ()
{
  return 0;
}

void ViewMethodEquidistant::SetRedisplay (bool redisp)
{
  m_redisplay = redisp;
}

void ViewMethodEquidistant::ResetShaders (std::string shadername)
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

void ViewMethodEquidistant::SetCubeWidth (float w)
{
}

void ViewMethodEquidistant::SetCubeHeight (float h)
{
}

void ViewMethodEquidistant::SetCubeDepth (float z)
{
}

float ViewMethodEquidistant::GetCubeWidth ()
{
  return 0.0f;
}

float ViewMethodEquidistant::GetCubeHeight ()
{
  return 0.0f;
}

float ViewMethodEquidistant::GetCubeDepth ()
{
  return 0.0f;
}

void ViewMethodEquidistant::SetGUICubeWidth (float w)
{
}

void ViewMethodEquidistant::SetGUICubeHeight (float h)
{
}

void ViewMethodEquidistant::SetGUICubeDepth (float z)
{
}

float ViewMethodEquidistant::GetGUICubeWidth ()
{
  return 0.0f;
}

float ViewMethodEquidistant::GetGUICubeHeight ()
{
  return 0.0f;
}

float ViewMethodEquidistant::GetGUICubeDepth ()
{
  return 0.0f;
}

void ViewMethodEquidistant::ChangeCurrentShaderProgram (int shader_id)
{

  CleanViewer ();
  BuildViewer ();
}

void ViewMethodEquidistant::UseDoublePrecision (bool usedoubleprec)
{
  CleanViewer ();
  BuildViewer ();
}