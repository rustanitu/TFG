#include "ERN2DViewMethod.h"

ERN2DViewMethod::ERN2DViewMethod ()
: ViewMethod (VRVIEWS::ERN)
{
  m_volumename = "";
  m_trasnferfunctionname = "";

  m_redisplay = true;
  m_auto_redisplay = false;
  m_mousepressed = false;
}

ERN2DViewMethod::~ERN2DViewMethod ()
{}

int ERN2DViewMethod::Idle_Action_CB (Ihandle* cnv_renderer)
{
  IupGLMakeCurrent (cnv_renderer);
  if (m_redisplay || m_auto_redisplay)
  {
    if (m_outdated)
    {
      if (!m_renderer.m_glsl_preintegrated_texture || Viewer::Instance ()->m_volumename.compare (m_volumename) != 0)
      {
        m_volumename = Viewer::Instance ()->m_volumename;
        m_renderer.ReloadVolume (Viewer::Instance ()->m_volume, Viewer::Instance ()->m_transfer_function);
      }

      UpdateIupUserInterface ();
      m_outdated = false;
    }

    if (m_renderer.m_glsl_preintegrated_texture)
    {
      m_renderer.Render (Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);
      IupGLSwapBuffers (cnv_renderer);
    }
    m_redisplay = false;
  }
  return IUP_DEFAULT;
}

int ERN2DViewMethod::Keyboard_CB (Ihandle *ih, int c, int press)
{
  if (press == 0)
  {
    switch (c)
    {
    case K_R:
    case K_r:
      m_redisplay = true;
      break;
    case K_T:
    case K_t:
      m_renderer.ReloadShaders ();
      break;
    case K_M:
    case K_m:
      m_renderer.SetYRotation (0.0f);
      m_renderer.SetXRotation (0.0f);
      m_redisplay = true;
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

int ERN2DViewMethod::Button_CB (Ihandle* ih, int button, int pressed, int x, int y, char* status)
{
  m_renderer.MouseButtonCB (button, pressed, x, y, status);
  return IUP_DEFAULT;
}

int ERN2DViewMethod::Motion_CB (Ihandle *ih, int x, int y, char *status)
{
  m_renderer.MouseMotionCB (x, y, status);
  m_redisplay = true; 
  return IUP_DEFAULT;
}

int ERN2DViewMethod::Resize_CB (Ihandle *ih, int width, int height)
{
  m_renderer.Resize (width, height);
  MarkOutdated ();
  return IUP_DEFAULT;
}

void ERN2DViewMethod::CreateIupUserInterface ()
{
  m_gui.CreateIupInterface ();
}

void ERN2DViewMethod::UpdateIupUserInterface ()
{
  m_gui.SetInitXSlice (m_renderer.GetInitXSlice ());
  m_gui.SetLastXSlice (m_renderer.GetLastXSlice ());
  m_gui.SetInitYSlice (m_renderer.GetInitYSlice ());
  m_gui.SetLastYSlice (m_renderer.GetLastYSlice ());
  m_gui.SetInitZSlice (m_renderer.GetInitZSlice ());
  m_gui.SetLastZSlice (m_renderer.GetLastZSlice ());

}

void ERN2DViewMethod::ResetCamera ()
{
  m_renderer.SetYRotation (0.0f);
  m_renderer.SetXRotation (0.0f);
  m_redisplay = true;
}

void ERN2DViewMethod::BuildViewer ()
{
  if (!m_built)
  {
    m_renderer.CreateScene (Viewer::Instance ()->m_CurrentWidth,
      Viewer::Instance ()->m_CurrentHeight,
      Viewer::Instance ()->m_volume,
      Viewer::Instance ()->m_transfer_function->GenerateTexture_1D_RGBA ());
    MarkOutdated ();

    printf ("ERN2DViewMethod Built\n");
    m_built = true;
  }
}

void ERN2DViewMethod::CleanViewer ()
{
  if (m_built)
  {
    m_renderer.Destroy ();

    printf ("ERN2DViewMethod Cleaned\n");
    m_built = false;
  }
}

void ERN2DViewMethod::SetAutoRedisplay (bool autored)
{
  m_auto_redisplay = autored;
}

void ERN2DViewMethod::SetRedisplay (bool redisp)
{
  m_redisplay = redisp;
}

void ERN2DViewMethod::ResetShaders (std::string shadername)
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

void ERN2DViewMethod::ApplyInputSliceSizes ()
{
  m_renderer.ReloadVolume (Viewer::Instance ()->m_volume, false);
}

void ERN2DViewMethod::SetSliceSizes (int ix, int lx, int iy, int ly, int iz, int lz)
{
  m_renderer.SetSliceSizes (ix, lx, iy, ly, iz, lz);
}

int ERN2DViewMethod::GetInitXSlice ()
{
  return m_renderer.GetInitXSlice ();
}

int ERN2DViewMethod::GetLastXSlice ()
{
  return m_renderer.GetLastXSlice ();
}

int ERN2DViewMethod::GetInitYSlice ()
{
  return m_renderer.GetInitYSlice ();
}

int ERN2DViewMethod::GetLastYSlice ()
{
  return m_renderer.GetLastYSlice ();
}

int ERN2DViewMethod::GetInitZSlice ()
{
  return m_renderer.GetInitZSlice ();
}

int ERN2DViewMethod::GetLastZSlice ()
{
  return m_renderer.GetLastZSlice ();
}

void ERN2DViewMethod::SetInitXSlice (int value)
{
  m_renderer.SetInitXSlice (value);
  m_gui.SetInitXSlice (m_renderer.GetInitXSlice ());
}

void ERN2DViewMethod::SetLastXSlice (int value)
{
  m_renderer.SetLastXSlice (value);
  m_gui.SetLastXSlice (m_renderer.GetLastXSlice ());
}

void ERN2DViewMethod::SetInitYSlice (int value)
{
  m_renderer.SetInitYSlice (value);
  m_gui.SetInitYSlice (m_renderer.GetInitYSlice ());
}

void ERN2DViewMethod::SetLastYSlice (int value)
{
  m_renderer.SetLastYSlice (value);
  m_gui.SetLastYSlice (m_renderer.GetLastYSlice ());
}

void ERN2DViewMethod::SetInitZSlice (int value)
{
  m_renderer.SetInitZSlice (value);
  m_gui.SetInitZSlice (m_renderer.GetInitZSlice ());
}

void ERN2DViewMethod::SetLastZSlice (int value)
{
  m_renderer.SetLastZSlice (value);
  m_gui.SetLastZSlice (m_renderer.GetLastZSlice ());
}

void ERN2DViewMethod::AutoModeling ()
{
  m_renderer.AutoModeling (Viewer::Instance ()->m_volume);
  m_renderer.ResetGeometry ();
  m_renderer.Resize (Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);
}

void ERN2DViewMethod::ApplyModeling (float xw, float yh, float zd)
{
  m_renderer.ApplyModeling (xw, yh, zd);
}

void ERN2DViewMethod::SetCubeWidth (float w)
{
  m_renderer.SetCubeWidth (w);
}

void ERN2DViewMethod::SetCubeHeight (float h)
{
  m_renderer.SetCubeHeight (h);
}

void ERN2DViewMethod::SetCubeDepth (float z)
{
  m_renderer.SetCubeDepth (z);
}

float ERN2DViewMethod::GetCubeWidth ()
{
  return m_renderer.GetCubeWidth ();
}

float ERN2DViewMethod::GetCubeHeight ()
{
  return m_renderer.GetCubeHeight ();
}

float ERN2DViewMethod::GetCubeDepth ()
{
  return m_renderer.GetCubeDepth ();
}

void ERN2DViewMethod::SetGUICubeWidth (float w)
{
  m_gui.m_gui_cubewidth = w;
  m_gui.UpdateCubeSizes ();
}

void ERN2DViewMethod::SetGUICubeHeight (float h)
{
  m_gui.m_gui_cubeheight = h;
  m_gui.UpdateCubeSizes ();
}

void ERN2DViewMethod::SetGUICubeDepth (float z)
{
  m_gui.m_gui_cubedepth = z;
  m_gui.UpdateCubeSizes ();
}

float ERN2DViewMethod::GetGUICubeWidth ()
{
  return m_gui.m_gui_cubewidth;
}

float ERN2DViewMethod::GetGUICubeHeight ()
{
  return m_gui.m_gui_cubeheight;
}

float ERN2DViewMethod::GetGUICubeDepth ()
{
  return m_gui.m_gui_cubedepth;
}
