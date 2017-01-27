#include "ERNViewMethod.h"

ERNViewMethod::ERNViewMethod ()
: ViewMethod (VRVIEWS::ERN)
{
  m_volumename = "";
  m_trasnferfunctionname = "";

  m_redisplay = true;
  m_auto_redisplay = false;
  m_mousepressed = false;
}

ERNViewMethod::~ERNViewMethod ()
{}

int ERNViewMethod::Idle_Action_CB (Ihandle* cnv_renderer)
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
      if (m_renderer.m_glsl_use_gradient == 1 && !m_renderer.m_glsl_gradient)
        SetUseGradient (0);
      m_renderer.Render (Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);
      IupGLSwapBuffers (cnv_renderer);
    }
    m_redisplay = false;
  }
  return IUP_DEFAULT;
}

int ERNViewMethod::Keyboard_CB (Ihandle *ih, int c, int press)
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
    case K_1:
      m_renderer.m_render = 0;
      m_redisplay = true;
      m_gui.SetVisualizationModel (m_renderer.m_render);
      break;
    case K_2:
      m_renderer.m_render = 1;
      m_redisplay = true;
      m_gui.SetVisualizationModel (m_renderer.m_render);
      break;
    case K_G:
    case K_g:
      m_renderer.ResetUseGradient ((m_renderer.m_glsl_use_gradient + 1) % 2, Viewer::Instance ()->m_volume);
      m_gui.SetGradientToggle (m_renderer.m_glsl_use_gradient);
      m_redisplay = true;
      break;
    case K_SP:
      m_renderer.m_iterate = !m_renderer.m_iterate;
      m_auto_redisplay = m_renderer.m_iterate;
      m_gui.SetIterateToggle (m_renderer.m_iterate);
      break;
    case K_A:
    case K_a:
      m_renderer.ResetSamples (m_renderer.m_glsl_samples / 2);
      m_gui.SetNumberOfSamples (m_renderer.m_glsl_samples);
      m_redisplay = true;
      break;
    case K_S:
    case K_s:
      m_renderer.ResetSamples (m_renderer.m_glsl_samples * 2);
      m_gui.SetNumberOfSamples (m_renderer.m_glsl_samples);
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

int ERNViewMethod::Button_CB (Ihandle* ih, int button, int pressed, int x, int y, char* status)
{
  m_mousepressed = pressed == 1 ? true : false;
  if (m_mousepressed)
  {
    m_motion_x = x;
    m_motion_y = y;
  }
  return IUP_DEFAULT;
}

int ERNViewMethod::Motion_CB (Ihandle *ih, int x, int y, char *status)
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

int ERNViewMethod::Resize_CB (Ihandle *ih, int width, int height)
{
  m_renderer.Resize (width, height);
  MarkOutdated ();
  return IUP_DEFAULT;
}

void ERNViewMethod::CreateIupUserInterface ()
{
  m_gui.CreateIupInterface ();
}

void ERNViewMethod::UpdateIupUserInterface ()
{
  m_gui.SetVisualizationModel (m_renderer.m_render);
  m_gui.SetNumberOfSamples (m_renderer.m_glsl_samples);
  m_gui.SetGradientToggle (m_renderer.m_glsl_use_gradient);
  m_gui.SetIterateToggle (m_renderer.m_iterate);

  m_gui.SetInitXSlice (m_renderer.GetInitXSlice ());
  m_gui.SetLastXSlice (m_renderer.GetLastXSlice ());
  m_gui.SetInitYSlice (m_renderer.GetInitYSlice ());
  m_gui.SetLastYSlice (m_renderer.GetLastYSlice ());
  m_gui.SetInitZSlice (m_renderer.GetInitZSlice ());
  m_gui.SetLastZSlice (m_renderer.GetLastZSlice ());

}

void ERNViewMethod::ResetCamera ()
{
  m_renderer.SetYRotation (0.0f);
  m_renderer.SetXRotation (0.0f);
  m_redisplay = true;
}

void ERNViewMethod::BuildViewer ()
{
  if (!m_built)
  {
    m_renderer.CreateScene (Viewer::Instance ()->m_CurrentWidth,
      Viewer::Instance ()->m_CurrentHeight,
      Viewer::Instance ()->m_volume,
      Viewer::Instance ()->m_transfer_function->GenerateTexture_RGBA ());
    MarkOutdated ();

    printf ("ERNViewMethod Built\n");
    m_built = true;
  }
}

void ERNViewMethod::CleanViewer ()
{
  if (m_built)
  {
    m_renderer.Destroy ();

    printf ("ERNViewMethod Cleaned\n");
    m_built = false;
  }
}

void ERNViewMethod::SetRenderMode (int rendermode)
{
  m_renderer.m_render = rendermode;
}

void ERNViewMethod::SetSamples (int samples)
{
  m_renderer.ResetSamples (samples);
}

int ERNViewMethod::GetSamples ()
{
  return m_renderer.m_glsl_samples;
}

void ERNViewMethod::SetUseGradient (int state)
{
  m_renderer.ResetUseGradient (state % 2, Viewer::Instance ()->m_volume);
}

int ERNViewMethod::GetUseGradient ()
{
  return m_renderer.m_glsl_use_gradient;
}

void ERNViewMethod::SetIterate (bool iter)
{
  m_renderer.m_iterate = iter;
}

bool ERNViewMethod::GetIterate ()
{
  return m_renderer.m_iterate;
}

void ERNViewMethod::SetAutoRedisplay (bool autored)
{
  m_auto_redisplay = autored;
}

void ERNViewMethod::SetRedisplay (bool redisp)
{
  m_redisplay = redisp;
}

void ERNViewMethod::ResetShaders (std::string shadername)
{
  m_renderer.ResetShaders (shadername);

  m_renderer.Destroy ();

  m_renderer.CreateScene (
    Viewer::Instance ()->m_CurrentWidth,
    Viewer::Instance ()->m_CurrentHeight,
    Viewer::Instance ()->m_volume,
    Viewer::Instance ()->m_transfer_function->GenerateTexture_RGBA (),
    false);
  MarkOutdated ();
}

void ERNViewMethod::ApplyInputSliceSizes ()
{
  m_renderer.ReloadVolume (Viewer::Instance ()->m_volume, false);
}

void ERNViewMethod::SetSliceSizes (int ix, int lx, int iy, int ly, int iz, int lz)
{
  m_renderer.SetSliceSizes (ix, lx, iy, ly, iz, lz);
}

int ERNViewMethod::GetInitXSlice ()
{
  return m_renderer.GetInitXSlice ();
}

int ERNViewMethod::GetLastXSlice ()
{
  return m_renderer.GetLastXSlice ();
}

int ERNViewMethod::GetInitYSlice ()
{
  return m_renderer.GetInitYSlice ();
}

int ERNViewMethod::GetLastYSlice ()
{
  return m_renderer.GetLastYSlice ();
}

int ERNViewMethod::GetInitZSlice ()
{
  return m_renderer.GetInitZSlice ();
}

int ERNViewMethod::GetLastZSlice ()
{
  return m_renderer.GetLastZSlice ();
}

void ERNViewMethod::SetInitXSlice (int value)
{
  m_renderer.SetInitXSlice (value);
  m_gui.SetInitXSlice (m_renderer.GetInitXSlice ());
}

void ERNViewMethod::SetLastXSlice (int value)
{
  m_renderer.SetLastXSlice (value);
  m_gui.SetLastXSlice (m_renderer.GetLastXSlice ());
}

void ERNViewMethod::SetInitYSlice (int value)
{
  m_renderer.SetInitYSlice (value);
  m_gui.SetInitYSlice (m_renderer.GetInitYSlice ());
}

void ERNViewMethod::SetLastYSlice (int value)
{
  m_renderer.SetLastYSlice (value);
  m_gui.SetLastYSlice (m_renderer.GetLastYSlice ());
}

void ERNViewMethod::SetInitZSlice (int value)
{
  m_renderer.SetInitZSlice (value);
  m_gui.SetInitZSlice (m_renderer.GetInitZSlice ());
}

void ERNViewMethod::SetLastZSlice (int value)
{
  m_renderer.SetLastZSlice (value);
  m_gui.SetLastZSlice (m_renderer.GetLastZSlice ());
}

void ERNViewMethod::AutoModeling ()
{
  m_renderer.AutoModeling (Viewer::Instance ()->m_volume);
  m_renderer.ResetGeometry ();
  m_renderer.Resize (Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);
}

void ERNViewMethod::ApplyModeling (float xw, float yh, float zd)
{
  m_renderer.ApplyModeling (xw, yh, zd);
}

void ERNViewMethod::SetCubeWidth (float w)
{
  m_renderer.SetCubeWidth (w);
}

void ERNViewMethod::SetCubeHeight (float h)
{
  m_renderer.SetCubeHeight (h);
}

void ERNViewMethod::SetCubeDepth (float z)
{
  m_renderer.SetCubeDepth (z);
}

float ERNViewMethod::GetCubeWidth ()
{
  return m_renderer.GetCubeWidth ();
}

float ERNViewMethod::GetCubeHeight ()
{
  return m_renderer.GetCubeHeight ();
}

float ERNViewMethod::GetCubeDepth ()
{
  return m_renderer.GetCubeDepth ();
}

void ERNViewMethod::SetGUICubeWidth (float w)
{
  m_gui.m_gui_cubewidth = w;
  m_gui.UpdateCubeSizes ();
}

void ERNViewMethod::SetGUICubeHeight (float h)
{
  m_gui.m_gui_cubeheight = h;
  m_gui.UpdateCubeSizes ();
}

void ERNViewMethod::SetGUICubeDepth (float z)
{
  m_gui.m_gui_cubedepth = z;
  m_gui.UpdateCubeSizes ();
}

float ERNViewMethod::GetGUICubeWidth ()
{
  return m_gui.m_gui_cubewidth;
}

float ERNViewMethod::GetGUICubeHeight ()
{
  return m_gui.m_gui_cubeheight;
}

float ERNViewMethod::GetGUICubeDepth ()
{
  return m_gui.m_gui_cubedepth;
}

void ERNViewMethod::ReloadTransferFunction()
{
  m_renderer.ReloadTransferFunction(Viewer::Instance()->m_transfer_function);
  UpdateIupUserInterface();
}
