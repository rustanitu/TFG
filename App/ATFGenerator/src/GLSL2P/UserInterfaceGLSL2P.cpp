#include "UserInterfaceGLSL2P.h"

#include "../Viewer.h"
#include "ViewMethodGLSL2P.h"

#include <GL/glew.h>
#include <ctime>

#include <glutils/GLShader.h>
#include <glutils/GLUtils.h>
#include <math/MUtils.h>
#include <volrend/Volume.h>
#include <volrend/TransferFunction.h>
#include <lqc/lqcdefines.h>

int UserInterfaceGLSL2P::Callback_tgl_UseDoublePrecision (Ihandle* ih, int state)
{
  ViewMethodGLSL2P* vmethod = (ViewMethodGLSL2P*)Viewer::Instance ()->m_viewmethods[VRVIEWS::GLSL2P];
  vmethod->UseDoublePrecision (state == 1 ? true : false);
  return IUP_DEFAULT;
}

int UserInterfaceGLSL2P::ToggleCB_GLSLITERATE (Ihandle* ih, int state)
{
  ViewMethodGLSL2P* vmethod = (ViewMethodGLSL2P*)Viewer::Instance ()->m_viewmethods[VRVIEWS::GLSL2P];
  vmethod->SetIterate (state == 1 ? true : false);
  vmethod->SetAutoRedisplay (vmethod->GetIterate ());
  return IUP_DEFAULT;
}

int UserInterfaceGLSL2P::SpinCB_InitSlicesX (Ihandle* ih, int increment)
{
  ViewMethodGLSL2P* vmethod = (ViewMethodGLSL2P*)Viewer::Instance ()->m_viewmethods[VRVIEWS::GLSL2P];
  if (increment == 1)
    vmethod->SetInitXSlice (vmethod->GetInitXSlice () + 1);
  else if (increment == -1)
    vmethod->SetInitXSlice (vmethod->GetInitXSlice () - 1);

  return IUP_DEFAULT;
}

int UserInterfaceGLSL2P::SpinCB_LastSlicesX (Ihandle* ih, int increment)
{
  ViewMethodGLSL2P* vmethod = (ViewMethodGLSL2P*)Viewer::Instance ()->m_viewmethods[VRVIEWS::GLSL2P];
  if (increment == 1)
    vmethod->SetLastXSlice (vmethod->GetLastXSlice () + 1);
  else if (increment == -1)
    vmethod->SetLastXSlice (vmethod->GetLastXSlice () - 1);

  return IUP_DEFAULT;
}

int UserInterfaceGLSL2P::SpinCB_InitSlicesY (Ihandle* ih, int increment)
{
  ViewMethodGLSL2P* vmethod = (ViewMethodGLSL2P*)Viewer::Instance ()->m_viewmethods[VRVIEWS::GLSL2P];
  if (increment == 1)
    vmethod->SetInitYSlice (vmethod->GetInitYSlice () + 1);
  else if (increment == -1)
    vmethod->SetInitYSlice (vmethod->GetInitYSlice () - 1);

  return IUP_DEFAULT;
}

int UserInterfaceGLSL2P::SpinCB_LastSlicesY (Ihandle* ih, int increment)
{
  ViewMethodGLSL2P* vmethod = (ViewMethodGLSL2P*)Viewer::Instance ()->m_viewmethods[VRVIEWS::GLSL2P];
  if (increment == 1)
    vmethod->SetLastYSlice (vmethod->GetLastYSlice () + 1);
  else if (increment == -1)
    vmethod->SetLastYSlice (vmethod->GetLastYSlice () - 1);

  return IUP_DEFAULT;
}

int UserInterfaceGLSL2P::SpinCB_InitSlicesZ (Ihandle* ih, int increment)
{
  ViewMethodGLSL2P* vmethod = (ViewMethodGLSL2P*)Viewer::Instance ()->m_viewmethods[VRVIEWS::GLSL2P];
  if (increment == 1)
    vmethod->SetInitZSlice (vmethod->GetInitZSlice () + 1);
  else if (increment == -1)
    vmethod->SetInitZSlice (vmethod->GetInitZSlice () - 1);

  return IUP_DEFAULT;
}

int UserInterfaceGLSL2P::SpinCB_LastSlicesZ (Ihandle* ih, int increment)
{
  ViewMethodGLSL2P* vmethod = (ViewMethodGLSL2P*)Viewer::Instance ()->m_viewmethods[VRVIEWS::GLSL2P];
  if (increment == 1)
    vmethod->SetLastZSlice (vmethod->GetLastZSlice () + 1);
  else if (increment == -1)
    vmethod->SetLastZSlice (vmethod->GetLastZSlice () - 1);

  return IUP_DEFAULT;
}

int UserInterfaceGLSL2P::ButtonCB_ReloadTextures (Ihandle* ih)
{
  ViewMethodGLSL2P* vmethod = (ViewMethodGLSL2P*)Viewer::Instance ()->m_viewmethods[VRVIEWS::GLSL2P];
  vmethod->ApplyInputSliceSizes ();
  return IUP_DEFAULT;
}

int UserInterfaceGLSL2P::ButtonCB_AutoModeling (Ihandle* ih)
{
  ViewMethodGLSL2P* vmethod = (ViewMethodGLSL2P*)Viewer::Instance ()->m_viewmethods[VRVIEWS::GLSL2P];
  vmethod->AutoModeling ();
  vmethod->MarkOutdated ();
  return IUP_DEFAULT;
}

UserInterfaceGLSL2P::UserInterfaceGLSL2P ()
{
  m_gui_cubewidth = 1.0f;
  m_gui_cubeheight = 1.0f;
  m_gui_cubedepth = 1.0f;
}

UserInterfaceGLSL2P::~UserInterfaceGLSL2P ()
{

}

void UserInterfaceGLSL2P::CreateIupInterface ()
{
  ViewMethodGLSL2P* vmethod = (ViewMethodGLSL2P*)Viewer::Instance ()->m_viewmethods[VRVIEWS::GLSL2P];

  m_iup_tgl_usedoubleprecision = IupToggle ("Usar dupla precis�o", NULL);
  IupSetCallback (m_iup_tgl_usedoubleprecision, "ACTION", (Icallback)Callback_tgl_UseDoublePrecision);

  m_iup_toggle_iterate = IupToggle ("Rotacionar modelo (' ')", NULL);
  IupSetAttribute (m_iup_toggle_iterate, "VALUE", vmethod->GetIterate () ? "YES" : "NO");
  IupSetCallback (m_iup_toggle_iterate, "ACTION", (Icallback)ToggleCB_GLSLITERATE);

  m_iup_label_slicesx = IupLabel ("X: ");
  m_iup_spin_initslicex = IupSpinbox (IupText (NULL));
  IupSetCallback (m_iup_spin_initslicex, "SPIN_CB", (Icallback)UserInterfaceGLSL2P::SpinCB_InitSlicesX);
  m_iup_spin_lastslicex = IupSpinbox (IupText (NULL));
  IupSetCallback (m_iup_spin_lastslicex, "SPIN_CB", (Icallback)UserInterfaceGLSL2P::SpinCB_LastSlicesX);
  m_iup_hbox_slicex = IupHbox (m_iup_label_slicesx, m_iup_spin_initslicex, m_iup_spin_lastslicex, NULL);

  m_iup_label_slicesy = IupLabel ("Y: ");
  m_iup_spin_initslicey = IupSpinbox (IupText (NULL));
  IupSetCallback (m_iup_spin_initslicey, "SPIN_CB", (Icallback)UserInterfaceGLSL2P::SpinCB_InitSlicesY);
  m_iup_spin_lastslicey = IupSpinbox (IupText (NULL));
  IupSetCallback (m_iup_spin_lastslicey, "SPIN_CB", (Icallback)UserInterfaceGLSL2P::SpinCB_LastSlicesY);
  m_iup_hbox_slicey = IupHbox (m_iup_label_slicesy, m_iup_spin_initslicey, m_iup_spin_lastslicey, NULL);

  m_iup_label_slicesz = IupLabel ("Z: ");
  m_iup_spin_initslicez = IupSpinbox (IupText (NULL));
  IupSetCallback (m_iup_spin_initslicez, "SPIN_CB", (Icallback)UserInterfaceGLSL2P::SpinCB_InitSlicesZ);
  m_iup_spin_lastslicez = IupSpinbox (IupText (NULL));
  IupSetCallback (m_iup_spin_lastslicez, "SPIN_CB", (Icallback)UserInterfaceGLSL2P::SpinCB_LastSlicesZ);
  m_iup_hbox_slicez = IupHbox (m_iup_label_slicesz, m_iup_spin_initslicez, m_iup_spin_lastslicez, NULL);

  m_iup_button_slicesizes = IupButton ("Aplicar", NULL);
  IupSetCallback (m_iup_button_slicesizes, "ACTION", (Icallback)UserInterfaceGLSL2P::ButtonCB_ReloadTextures);

  m_iup_label_volumesizes = IupLabel ("Geometria:");
  m_iup_button_volumesizes = IupButton ("Redimensionar Geometria", NULL);
  IupSetCallback (m_iup_button_volumesizes, "ACTION", (Icallback)UserInterfaceGLSL2P::ButtonCB_AutoModeling);
  //m_iup_hbox_volumesizes = IupHbox (m_iup_label_volumesizes, m_iup_button_volumesizes, NULL);
  
  m_iup_text_volumesizes = IupText (NULL);
  IupSetAttribute (m_iup_text_volumesizes, "BORDER", "NO");
  IupSetAttribute (m_iup_text_volumesizes, "READONLY", "YES");
  IupSetAttribute (m_iup_text_volumesizes, "VALUE", "[1.0, 1.0, 1.0]");

  vmethod->SetIupUserInterface (
    IupVbox (
    m_iup_button_volumesizes,
    //m_iup_tgl_usedoubleprecision,
    //m_iup_toggle_iterate,
    IupLabel ("Recorte:"),
    m_iup_hbox_slicex, 
    m_iup_hbox_slicey, 
    m_iup_hbox_slicez, 
    m_iup_button_slicesizes,
    //m_iup_hbox_volumesizes,
    //m_iup_text_volumesizes,
    NULL));
}

void UserInterfaceGLSL2P::SetIterateToggle (bool itoggle)
{
  IupSetAttribute (m_iup_toggle_iterate, "VALUE", itoggle ? "YES" : "NO");
}

void UserInterfaceGLSL2P::SetInitXSlice (int initx)
{
  char s[20];
  sprintf (s, "%d", initx);
  IupSetAttribute (IupGetChild (m_iup_spin_initslicex, 1), "VALUE", s);
}

void UserInterfaceGLSL2P::SetLastXSlice (int lastx)
{
  char s[20];
  sprintf (s, "%d", lastx);
  IupSetAttribute (IupGetChild (m_iup_spin_lastslicex, 1), "VALUE", s);
}

void UserInterfaceGLSL2P::SetInitYSlice (int inity)
{
  char s[20];
  sprintf (s, "%d", inity);
  IupSetAttribute (IupGetChild (m_iup_spin_initslicey, 1), "VALUE", s);
}

void UserInterfaceGLSL2P::SetLastYSlice (int lasty)
{
  char s[20];
  sprintf (s, "%d", lasty);
  IupSetAttribute (IupGetChild (m_iup_spin_lastslicey, 1), "VALUE", s);
}

void UserInterfaceGLSL2P::SetInitZSlice (int initz)
{
  char s[20];
  sprintf (s, "%d", initz);
  IupSetAttribute (IupGetChild (m_iup_spin_initslicez, 1), "VALUE", s);
}

void UserInterfaceGLSL2P::SetLastZSlice (int lastz)
{
  char s[20];
  sprintf (s, "%d", lastz);
  IupSetAttribute (IupGetChild (m_iup_spin_lastslicez, 1), "VALUE", s);
}

void UserInterfaceGLSL2P::UpdateCubeSizes ()
{
  char s[100];
  sprintf (s, "[%.4f, %.4f, %.4f]", m_gui_cubewidth, m_gui_cubeheight, m_gui_cubedepth);
  IupSetAttribute (m_iup_text_volumesizes, "VALUE", s);
}
