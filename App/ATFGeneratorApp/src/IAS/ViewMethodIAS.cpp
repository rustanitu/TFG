#include "ViewMethodIAS.h"

ViewMethodIAS::ViewMethodIAS ()
  : GLSLViewMethod(VRVIEWS::IAS)
{
  m_volumename = "";
  m_trasnferfunctionname = "";

  m_redisplay = true;
  m_mousepressed = false;
}

ViewMethodIAS::~ViewMethodIAS ()
{}

int ViewMethodIAS::Idle_Action_CB (Ihandle* cnv_renderer)
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

int ViewMethodIAS::Keyboard_CB (Ihandle *ih, int c, int press)
{
  if (press == 0)
  {
    switch (c)
    {
      case K_P:
      case K_p:
        Viewer::Instance ()->SaveSnapshot ("ias_screenshot.png");
        return IUP_DEFAULT;
      break;
      case K_R:
      case K_r:
        m_redisplay = true;
        break;
      case K_X:
      case K_x:
        m_renderer.ReloadShaders ();
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
        m_renderer.SetYRotation (0.0f);
        m_renderer.SetXRotation (0.0f);
        m_redisplay = true;
        break;
      case K_G:
      case K_g:
        m_renderer.ResetUseGradient ((m_renderer.m_glsl_use_gradient + 1) % 2, Viewer::Instance ()->m_volume);
        m_gui.SetGradientToggle (m_renderer.m_glsl_use_gradient);
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

int ViewMethodIAS::Button_CB (Ihandle* ih, int button, int pressed, int x, int y, char* status)
{
  m_mousepressed = pressed == 1 ? true : false;
  if (pressed == 1)
  {
    m_motion_x = x;
    m_motion_y = y;
  }
  else if (pressed == 0)
  {
    m_redisplay = true;
  }
  return IUP_DEFAULT;
}

int ViewMethodIAS::Motion_CB (Ihandle *ih, int x, int y, char *status)
{
  if (m_mousepressed)
  {
    int ydiff = m_motion_y - y;
    if (ydiff != 0)
    {
      m_renderer.SetXRotation (m_renderer.GetXRotation () + ydiff);
      m_motion_y = y;
      m_redisplay = true;
    }
    int xdiff = m_motion_x - x;
    if (xdiff != 0)
    {
      m_renderer.SetYRotation (m_renderer.GetYRotation () - xdiff);
      m_motion_x = x;
      m_redisplay = true;
    }
  }
  return IUP_DEFAULT;
}

int ViewMethodIAS::Resize_CB (Ihandle *ih, int width, int height)
{
  m_renderer.Resize (width, height);
  MarkOutdated ();
  return IUP_DEFAULT;
}

void ViewMethodIAS::CreateIupUserInterface ()
{
  m_gui.CreateIupInterface ();
}

void ViewMethodIAS::UpdateIupUserInterface ()
{
  m_gui.SetGradientToggle (m_renderer.m_glsl_use_gradient);

  m_gui.SetInitXSlice (m_renderer.GetInitXSlice ());
  m_gui.SetLastXSlice (m_renderer.GetLastXSlice ());
  m_gui.SetInitYSlice (m_renderer.GetInitYSlice ());
  m_gui.SetLastYSlice (m_renderer.GetLastYSlice ());
  m_gui.SetInitZSlice (m_renderer.GetInitZSlice ());
  m_gui.SetLastZSlice (m_renderer.GetLastZSlice ());

}

void ViewMethodIAS::ResetCamera ()
{
  m_renderer.SetYRotation (0.0f);
  m_renderer.SetXRotation (0.0f);
  m_redisplay = true;
}

void ViewMethodIAS::SaveCameraState (std::string filename)
{
  m_renderer.SaveInitCameraState (filename);
}

void ViewMethodIAS::LoadCameraState (std::string filename)
{
  m_renderer.LoadInitCameraState (filename);
}

void ViewMethodIAS::BuildViewer ()
{
  if (!m_built)
  {
    m_renderer.CreateScene (Viewer::Instance ()->m_CurrentWidth,
      Viewer::Instance ()->m_CurrentHeight,
      Viewer::Instance ()->m_volume,
      Viewer::Instance ()->m_transfer_function->GenerateTexture_1D_RGBA ());
    MarkOutdated ();
    
    printf ("ViewMethodIAS Built\n");
    m_built = true;
  }
}

void ViewMethodIAS::CleanViewer ()
{
  if (m_built)
  {
    m_renderer.Destroy ();

    printf ("ViewMethodIAS Cleaned\n");
    m_built = false;
  }
}

void ViewMethodIAS::SetUseGradient (int state)
{
  m_renderer.ResetUseGradient (state % 2, Viewer::Instance ()->m_volume);
}

int ViewMethodIAS::GetUseGradient ()
{
  return m_renderer.m_glsl_use_gradient;
}

void ViewMethodIAS::SetRedisplay (bool redisp)
{
  m_redisplay = redisp;
}

void ViewMethodIAS::ResetShaders (std::string shadername)
{
  m_renderer.ResetShaders (shadername);
  
  m_renderer.Destroy ();

  m_renderer.CreateScene (
    Viewer::Instance ()->m_CurrentWidth,
    Viewer::Instance ()->m_CurrentHeight,
    Viewer::Instance ()->m_volume,
    Viewer::Instance ()->m_transfer_function->GenerateTexture_1D_RGBA (),
    false);
  MarkOutdated ();
}

void ViewMethodIAS::ApplyInputSliceSizes ()
{
  m_renderer.ReloadVolume (Viewer::Instance ()->m_volume, false);
}

void ViewMethodIAS::SetSliceSizes (int ix, int lx, int iy, int ly, int iz, int lz)
{
  m_renderer.SetSliceSizes (ix, lx, iy, ly, iz, lz);
}

int ViewMethodIAS::GetInitXSlice ()
{
  return m_renderer.GetInitXSlice ();
}

int ViewMethodIAS::GetLastXSlice ()
{
  return m_renderer.GetLastXSlice ();
}

int ViewMethodIAS::GetInitYSlice ()
{
  return m_renderer.GetInitYSlice ();
}

int ViewMethodIAS::GetLastYSlice ()
{
  return m_renderer.GetLastYSlice ();
}

int ViewMethodIAS::GetInitZSlice ()
{
  return m_renderer.GetInitZSlice ();
}

int ViewMethodIAS::GetLastZSlice ()
{
  return m_renderer.GetLastZSlice ();
}

void ViewMethodIAS::SetInitXSlice (int value)
{
  m_renderer.SetInitXSlice (value);
  m_gui.SetInitXSlice (m_renderer.GetInitXSlice ());
}

void ViewMethodIAS::SetLastXSlice (int value)
{
  m_renderer.SetLastXSlice (value);
  m_gui.SetLastXSlice (m_renderer.GetLastXSlice ());
}

void ViewMethodIAS::SetInitYSlice (int value)
{
  m_renderer.SetInitYSlice (value);
  m_gui.SetInitYSlice (m_renderer.GetInitYSlice ());
}

void ViewMethodIAS::SetLastYSlice (int value)
{
  m_renderer.SetLastYSlice (value);
  m_gui.SetLastYSlice (m_renderer.GetLastYSlice ());
}

void ViewMethodIAS::SetInitZSlice (int value)
{
  m_renderer.SetInitZSlice (value);
  m_gui.SetInitZSlice (m_renderer.GetInitZSlice ());
}

void ViewMethodIAS::SetLastZSlice (int value)
{
  m_renderer.SetLastZSlice (value);
  m_gui.SetLastZSlice (m_renderer.GetLastZSlice ());
}

void ViewMethodIAS::AutoModeling ()
{
  m_renderer.AutoModeling (Viewer::Instance ()->m_volume);
  m_renderer.ResetGeometry ();
  m_renderer.Resize (Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);
}

void ViewMethodIAS::ApplyModeling (float xw, float yh, float zd)
{
  m_renderer.ApplyModeling (xw, yh, zd);
}

void ViewMethodIAS::SetCubeWidth (float w)
{
  m_renderer.SetCubeWidth (w);
}

void ViewMethodIAS::SetCubeHeight (float h)
{
  m_renderer.SetCubeHeight (h);
}

void ViewMethodIAS::SetCubeDepth (float z)
{
  m_renderer.SetCubeDepth (z);
}

float ViewMethodIAS::GetCubeWidth ()
{
  return m_renderer.GetCubeWidth ();
}

float ViewMethodIAS::GetCubeHeight ()
{
  return m_renderer.GetCubeHeight ();
}

float ViewMethodIAS::GetCubeDepth ()
{
  return m_renderer.GetCubeDepth ();
}

void ViewMethodIAS::SetGUICubeWidth (float w)
{
  m_gui.m_gui_cubewidth = w;
  m_gui.UpdateCubeSizes ();
}

void ViewMethodIAS::SetGUICubeHeight (float h)
{
  m_gui.m_gui_cubeheight = h;
  m_gui.UpdateCubeSizes ();
}

void ViewMethodIAS::SetGUICubeDepth (float z)
{
  m_gui.m_gui_cubedepth = z;
  m_gui.UpdateCubeSizes ();
}

float ViewMethodIAS::GetGUICubeWidth ()
{
  return m_gui.m_gui_cubewidth;
}

float ViewMethodIAS::GetGUICubeHeight ()
{
  return m_gui.m_gui_cubeheight;
}

float ViewMethodIAS::GetGUICubeDepth ()
{
  return m_gui.m_gui_cubedepth;
}

void ViewMethodIAS::ChangeCurrentShaderProgram (int shader_id)
{
  m_renderer.SHADER_ID = shader_id - 1;

  CleanViewer ();
  BuildViewer ();
}

void ViewMethodIAS::UseDoublePrecision (bool usedoubleprec)
{
  m_renderer.USE_DOUBLE_PRECISION = usedoubleprec;

  CleanViewer ();
  BuildViewer ();
}