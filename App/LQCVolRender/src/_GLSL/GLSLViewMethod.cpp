#include "GLSLViewMethod.h"

GLSLViewMethod::GLSLViewMethod ()
: ViewMethod (VRVIEWS::GLSL)
{
  m_volumename = "";
  m_trasnferfunctionname = "";

  m_redisplay = true;
  m_auto_redisplay = false;
  m_mousepressed = false;
}

GLSLViewMethod::~GLSLViewMethod ()
{}

int GLSLViewMethod::Idle_Action_CB (Ihandle* cnv_renderer)
{
  IupGLMakeCurrent (cnv_renderer);
  if (m_redisplay || m_auto_redisplay)
  {
    if (m_outdated)
    {
      if (!m_viewer.m_glsl_volume || Viewer::Instance ()->m_volumename.compare (m_volumename) != 0)
      {
        m_volumename = Viewer::Instance ()->m_volumename;
        m_viewer.ReloadVolume (Viewer::Instance ()->m_volume);
      }
      if (!m_viewer.m_glsl_transfer_function || Viewer::Instance ()->m_tf_name.compare (m_trasnferfunctionname) != 0)
      {
        m_trasnferfunctionname = Viewer::Instance ()->m_tf_name;
        m_viewer.ReloadTransferFunction (Viewer::Instance ()->m_transfer_function);
      }
      UpdateIupUserInterface ();
      m_outdated = false;
    }

    if (m_viewer.m_glsl_volume && m_viewer.m_glsl_transfer_function)
    {
      if (m_viewer.m_glsl_use_gradient == 1 && !m_viewer.m_glsl_gradient)
        SetUseGradient (0);
      m_viewer.Render (Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);
      IupGLSwapBuffers (cnv_renderer);
    }
    m_redisplay = false;
  }
  return IUP_DEFAULT;
}

int GLSLViewMethod::Keyboard_CB (Ihandle *ih, int c, int press)
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
        m_viewer.ReloadShaders ();
        break;
      case K_M:
      case K_m:
        m_viewer.SetYRotation(0.0f);
        m_viewer.SetXRotation(0.0f);
        m_redisplay = true;
        break;
      case K_1:
        m_viewer.m_render = 0;
        m_redisplay = true;
        m_gui.SetVisualizationModel (m_viewer.m_render);
        break;
      case K_2:
        m_viewer.m_render = 1;
        m_redisplay = true;
        m_gui.SetVisualizationModel (m_viewer.m_render);
        break;
      case K_G:
      case K_g:
        m_viewer.ResetUseGradient ((m_viewer.m_glsl_use_gradient + 1) % 2, Viewer::Instance()->m_volume);
        m_gui.SetGradientToggle (m_viewer.m_glsl_use_gradient);
        m_redisplay = true;
        break;
      case K_SP:
        m_viewer.m_iterate = !m_viewer.m_iterate;
        m_auto_redisplay = m_viewer.m_iterate;
        m_gui.SetIterateToggle (m_viewer.m_iterate);
        break;
      case K_A:
      case K_a:
        m_viewer.ResetSamples (m_viewer.m_glsl_samples / 2);
        m_gui.SetNumberOfSamples (m_viewer.m_glsl_samples);
        m_redisplay = true;
        break;
      case K_S:
      case K_s:
        m_viewer.ResetSamples (m_viewer.m_glsl_samples * 2);
        m_gui.SetNumberOfSamples (m_viewer.m_glsl_samples);
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

int GLSLViewMethod::Button_CB (Ihandle* ih, int button, int pressed, int x, int y, char* status)
{
  m_mousepressed = pressed == 1 ? true : false;
  if (m_mousepressed)
  {
    m_motion_x = x;
    m_motion_y = y;
  }
  return IUP_DEFAULT;
}

int GLSLViewMethod::Motion_CB (Ihandle *ih, int x, int y, char *status)
{
  if (m_mousepressed)
  {
    int ydiff = m_motion_y - y;
    if (ydiff != 0)
    {
      m_viewer.SetXRotation (m_viewer.GetXRotation() + ydiff);
      m_motion_y = y;
      m_redisplay = true;
    }
    int xdiff = m_motion_x - x;
    if (xdiff != 0)
    {
      m_viewer.SetYRotation (m_viewer.GetYRotation() - xdiff);
      m_motion_x = x;
      m_redisplay = true;
    }
  }
  return IUP_DEFAULT;
}

int GLSLViewMethod::Resize_CB (Ihandle *ih, int width, int height)
{
  m_viewer.Resize (width, height);
  MarkOutdated ();
  return IUP_DEFAULT;
}

void GLSLViewMethod::CreateIupUserInterface ()
{
  m_gui.CreateIupInterface ();
}

void GLSLViewMethod::UpdateIupUserInterface ()
{
  m_gui.SetVisualizationModel (m_viewer.m_render);
  m_gui.SetNumberOfSamples (m_viewer.m_glsl_samples);
  m_gui.SetGradientToggle (m_viewer.m_glsl_use_gradient);
  m_gui.SetIterateToggle (m_viewer.m_iterate);

  m_gui.SetInitXSlice (m_viewer.GetInitXSlice());
  m_gui.SetLastXSlice (m_viewer.GetLastXSlice ());
  m_gui.SetInitYSlice (m_viewer.GetInitYSlice ());
  m_gui.SetLastYSlice (m_viewer.GetLastYSlice ());
  m_gui.SetInitZSlice (m_viewer.GetInitZSlice ());
  m_gui.SetLastZSlice (m_viewer.GetLastZSlice ());

}

void GLSLViewMethod::ResetCamera ()
{
  m_viewer.SetYRotation (0.0f);
  m_viewer.SetXRotation (0.0f);
  m_redisplay = true;
}

void GLSLViewMethod::BuildViewer ()
{
  if (!m_built)
  {
    m_viewer.CreateScene (Viewer::Instance ()->m_CurrentWidth,
      Viewer::Instance ()->m_CurrentHeight,
      Viewer::Instance ()->m_volume,
      Viewer::Instance ()->m_transfer_function->GenerateTexture_1D_RGBA ());
    MarkOutdated ();
    
    printf ("GLSLViewMethod Built\n");
    m_built = true;
  }
}

void GLSLViewMethod::CleanViewer ()
{
  if (m_built)
  {
    m_viewer.Destroy ();

    printf ("GLSLViewMethod Cleaned\n");
    m_built = false;
  }
}

void GLSLViewMethod::SetRenderMode (int rendermode)
{
  m_viewer.m_render = rendermode;
}

void GLSLViewMethod::SetSamples (int samples)
{
  m_viewer.ResetSamples (samples);
}

int GLSLViewMethod::GetSamples ()
{
  return m_viewer.m_glsl_samples;
}

void GLSLViewMethod::SetUseGradient (int state)
{
  m_viewer.ResetUseGradient (state % 2, Viewer::Instance ()->m_volume);
}

int GLSLViewMethod::GetUseGradient ()
{
  return m_viewer.m_glsl_use_gradient;
}

void GLSLViewMethod::SetIterate (bool iter)
{
  m_viewer.m_iterate = iter;
}

bool GLSLViewMethod::GetIterate ()
{
  return m_viewer.m_iterate;
}

void GLSLViewMethod::SetAutoRedisplay (bool autored)
{
  m_auto_redisplay = autored;
}

void GLSLViewMethod::SetRedisplay (bool redisp)
{
  m_redisplay = redisp;
}

void GLSLViewMethod::ResetShaders (std::string shadername)
{
  m_viewer.ResetShaders (shadername);
  
  m_viewer.Destroy ();

  m_viewer.CreateScene (
    Viewer::Instance ()->m_CurrentWidth,
    Viewer::Instance ()->m_CurrentHeight,
    Viewer::Instance ()->m_volume,
    Viewer::Instance ()->m_transfer_function->GenerateTexture_1D_RGBA (),
    false);
  MarkOutdated ();
}

void GLSLViewMethod::ApplyInputSliceSizes ()
{
  m_viewer.ReloadVolume (Viewer::Instance()->m_volume, false);
}

void GLSLViewMethod::SetSliceSizes (int ix, int lx, int iy, int ly, int iz, int lz)
{
  m_viewer.SetSliceSizes (ix, lx, iy, ly, iz, lz);
}

int GLSLViewMethod::GetInitXSlice ()
{
  return m_viewer.GetInitXSlice ();
}

int GLSLViewMethod::GetLastXSlice ()
{
  return m_viewer.GetLastXSlice ();
}

int GLSLViewMethod::GetInitYSlice ()
{
  return m_viewer.GetInitYSlice ();
}

int GLSLViewMethod::GetLastYSlice ()
{
  return m_viewer.GetLastYSlice ();
}

int GLSLViewMethod::GetInitZSlice ()
{
  return m_viewer.GetInitZSlice ();
}

int GLSLViewMethod::GetLastZSlice ()
{
  return m_viewer.GetLastZSlice ();
}

void GLSLViewMethod::SetInitXSlice (int value)
{
  m_viewer.SetInitXSlice (value);
  m_gui.SetInitXSlice (m_viewer.GetInitXSlice());
}

void GLSLViewMethod::SetLastXSlice (int value)
{
  m_viewer.SetLastXSlice (value);
  m_gui.SetLastXSlice (m_viewer.GetLastXSlice ());
}

void GLSLViewMethod::SetInitYSlice (int value)
{
  m_viewer.SetInitYSlice (value);
  m_gui.SetInitYSlice (m_viewer.GetInitYSlice ());
}

void GLSLViewMethod::SetLastYSlice (int value)
{
  m_viewer.SetLastYSlice (value);
  m_gui.SetLastYSlice (m_viewer.GetLastYSlice ());
}

void GLSLViewMethod::SetInitZSlice (int value)
{
  m_viewer.SetInitZSlice (value);
  m_gui.SetInitZSlice (m_viewer.GetInitZSlice ());
}

void GLSLViewMethod::SetLastZSlice (int value)
{
  m_viewer.SetLastZSlice (value);
  m_gui.SetLastZSlice (m_viewer.GetLastZSlice ());
}

void GLSLViewMethod::AutoModeling ()
{
  m_viewer.AutoModeling (Viewer::Instance()->m_volume);
  m_viewer.ResetGeometry ();
  m_viewer.Resize (Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);
}

void GLSLViewMethod::ApplyModeling (float xw, float yh, float zd)
{
  m_viewer.ApplyModeling (xw, yh, zd);
}

void GLSLViewMethod::SetCubeWidth (float w)
{
  m_viewer.SetCubeWidth (w);
}

void GLSLViewMethod::SetCubeHeight (float h)
{
  m_viewer.SetCubeHeight (h);
}

void GLSLViewMethod::SetCubeDepth (float z)
{
  m_viewer.SetCubeDepth (z);
}

float GLSLViewMethod::GetCubeWidth ()
{
  return m_viewer.GetCubeWidth();
}

float GLSLViewMethod::GetCubeHeight ()
{
  return m_viewer.GetCubeHeight ();
}

float GLSLViewMethod::GetCubeDepth ()
{
  return m_viewer.GetCubeDepth ();
}

void GLSLViewMethod::SetGUICubeWidth (float w)
{
  m_gui.m_gui_cubewidth = w;
  m_gui.UpdateCubeSizes ();
}

void GLSLViewMethod::SetGUICubeHeight (float h)
{
  m_gui.m_gui_cubeheight = h;
  m_gui.UpdateCubeSizes ();
}

void GLSLViewMethod::SetGUICubeDepth (float z)
{
  m_gui.m_gui_cubedepth = z;
  m_gui.UpdateCubeSizes ();
}

float GLSLViewMethod::GetGUICubeWidth ()
{
  return m_gui.m_gui_cubewidth;
}

float GLSLViewMethod::GetGUICubeHeight ()
{
  return m_gui.m_gui_cubeheight;
}

float GLSLViewMethod::GetGUICubeDepth ()
{
  return m_gui.m_gui_cubedepth;
}
