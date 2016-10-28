#include "ERN2DUserInterface.h"

#include "../Viewer.h"
#include "ERN2DViewMethod.h"

#include <GL/glew.h>
#include <ctime>

#include <glutils/GLShader.h>
#include <glutils/GLUtils.h>
#include <math/MUtils.h>
#include <volrend/ScalarField.h>
#include <volrend/TransferFunction.h>
#include <lqc/lqcdefines.h>

int ERN2DUserInterface::ListCB_GLSLComposition (Ihandle* ih, char *text, int item, int state)
{
  if (state == 1)
  {
    ERN2DViewMethod* vmethod = (ERN2DViewMethod*)Viewer::Instance ()->m_viewmethods[VRVIEWS::ERN2D];
    vmethod->ResetShaders (text);
  }
  return IUP_DEFAULT;
}

int ERN2DUserInterface::SpinCB_InitSlicesX (Ihandle* ih, int increment)
{
  ERN2DViewMethod* vmethod = (ERN2DViewMethod*)Viewer::Instance ()->m_viewmethods[VRVIEWS::ERN2D];
  if (increment == 1)
    vmethod->SetInitXSlice (vmethod->GetInitXSlice () + 1);
  else if (increment == -1)
    vmethod->SetInitXSlice (vmethod->GetInitXSlice () - 1);

  return IUP_DEFAULT;
}

int ERN2DUserInterface::SpinCB_LastSlicesX (Ihandle* ih, int increment)
{
  ERN2DViewMethod* vmethod = (ERN2DViewMethod*)Viewer::Instance ()->m_viewmethods[VRVIEWS::ERN2D];
  if (increment == 1)
    vmethod->SetLastXSlice (vmethod->GetLastXSlice () + 1);
  else if (increment == -1)
    vmethod->SetLastXSlice (vmethod->GetLastXSlice () - 1);

  return IUP_DEFAULT;
}

int ERN2DUserInterface::SpinCB_InitSlicesY (Ihandle* ih, int increment)
{
  ERN2DViewMethod* vmethod = (ERN2DViewMethod*)Viewer::Instance ()->m_viewmethods[VRVIEWS::ERN2D];
  if (increment == 1)
    vmethod->SetInitYSlice (vmethod->GetInitYSlice () + 1);
  else if (increment == -1)
    vmethod->SetInitYSlice (vmethod->GetInitYSlice () - 1);

  return IUP_DEFAULT;
}

int ERN2DUserInterface::SpinCB_LastSlicesY (Ihandle* ih, int increment)
{
  ERN2DViewMethod* vmethod = (ERN2DViewMethod*)Viewer::Instance ()->m_viewmethods[VRVIEWS::ERN2D];
  if (increment == 1)
    vmethod->SetLastYSlice (vmethod->GetLastYSlice () + 1);
  else if (increment == -1)
    vmethod->SetLastYSlice (vmethod->GetLastYSlice () - 1);

  return IUP_DEFAULT;
}

int ERN2DUserInterface::SpinCB_InitSlicesZ (Ihandle* ih, int increment)
{
  ERN2DViewMethod* vmethod = (ERN2DViewMethod*)Viewer::Instance ()->m_viewmethods[VRVIEWS::ERN2D];
  if (increment == 1)
    vmethod->SetInitZSlice (vmethod->GetInitZSlice () + 1);
  else if (increment == -1)
    vmethod->SetInitZSlice (vmethod->GetInitZSlice () - 1);

  return IUP_DEFAULT;
}

int ERN2DUserInterface::SpinCB_LastSlicesZ (Ihandle* ih, int increment)
{
  ERN2DViewMethod* vmethod = (ERN2DViewMethod*)Viewer::Instance ()->m_viewmethods[VRVIEWS::ERN2D];
  if (increment == 1)
    vmethod->SetLastZSlice (vmethod->GetLastZSlice () + 1);
  else if (increment == -1)
    vmethod->SetLastZSlice (vmethod->GetLastZSlice () - 1);

  return IUP_DEFAULT;
}

int ERN2DUserInterface::ButtonCB_ReloadTextures (Ihandle* ih)
{
  ERN2DViewMethod* vmethod = (ERN2DViewMethod*)Viewer::Instance ()->m_viewmethods[VRVIEWS::ERN2D];
  vmethod->ApplyInputSliceSizes ();
  return IUP_DEFAULT;
}

int ERN2DUserInterface::ButtonCB_AutoModeling (Ihandle* ih)
{
  ERN2DViewMethod* vmethod = (ERN2DViewMethod*)Viewer::Instance ()->m_viewmethods[VRVIEWS::ERN2D];
  vmethod->AutoModeling ();
  vmethod->MarkOutdated ();
  return IUP_DEFAULT;
}

ERN2DUserInterface::ERN2DUserInterface ()
{
  m_gui_cubewidth = 1.0f;
  m_gui_cubeheight = 1.0f;
  m_gui_cubedepth = 1.0f;
}

ERN2DUserInterface::~ERN2DUserInterface ()
{

}

void ERN2DUserInterface::CreateIupInterface ()
{
  ERN2DViewMethod* vmethod = (ERN2DViewMethod*)Viewer::Instance ()->m_viewmethods[VRVIEWS::ERN2D];

  m_iup_label_slicesx = IupLabel ("X: ");
  m_iup_spin_initslicex = IupSpinbox (IupText (NULL));
  IupSetCallback (m_iup_spin_initslicex, "SPIN_CB", (Icallback)ERN2DUserInterface::SpinCB_InitSlicesX);
  m_iup_spin_lastslicex = IupSpinbox (IupText (NULL));
  IupSetCallback (m_iup_spin_lastslicex, "SPIN_CB", (Icallback)ERN2DUserInterface::SpinCB_LastSlicesX);
  m_iup_hbox_slicex = IupHbox (m_iup_label_slicesx, m_iup_spin_initslicex, m_iup_spin_lastslicex, NULL);

  m_iup_label_slicesy = IupLabel ("Y: ");
  m_iup_spin_initslicey = IupSpinbox (IupText (NULL));
  IupSetCallback (m_iup_spin_initslicey, "SPIN_CB", (Icallback)ERN2DUserInterface::SpinCB_InitSlicesY);
  m_iup_spin_lastslicey = IupSpinbox (IupText (NULL));
  IupSetCallback (m_iup_spin_lastslicey, "SPIN_CB", (Icallback)ERN2DUserInterface::SpinCB_LastSlicesY);
  m_iup_hbox_slicey = IupHbox (m_iup_label_slicesy, m_iup_spin_initslicey, m_iup_spin_lastslicey, NULL);

  m_iup_label_slicesz = IupLabel ("Z: ");
  m_iup_spin_initslicez = IupSpinbox (IupText (NULL));
  IupSetCallback (m_iup_spin_initslicez, "SPIN_CB", (Icallback)ERN2DUserInterface::SpinCB_InitSlicesZ);
  m_iup_spin_lastslicez = IupSpinbox (IupText (NULL));
  IupSetCallback (m_iup_spin_lastslicez, "SPIN_CB", (Icallback)ERN2DUserInterface::SpinCB_LastSlicesZ);
  m_iup_hbox_slicez = IupHbox (m_iup_label_slicesz, m_iup_spin_initslicez, m_iup_spin_lastslicez, NULL);

  m_iup_button_slicesizes = IupButton ("Aplicar", NULL);
  IupSetAttribute (m_iup_button_slicesizes, "FGCOLOR", "255 0 0");;
  IupSetCallback (m_iup_button_slicesizes, "ACTION", (Icallback)ERN2DUserInterface::ButtonCB_ReloadTextures);

  m_iup_label_volumesizes = IupLabel ("Geometria:");
  m_iup_button_volumesizes = IupButton ("AutoModelar", NULL);
  IupSetAttribute (m_iup_button_volumesizes, "FGCOLOR", "255 0 0");
  IupSetCallback (m_iup_button_volumesizes, "ACTION", (Icallback)ERN2DUserInterface::ButtonCB_AutoModeling);
  m_iup_hbox_volumesizes = IupHbox (m_iup_label_volumesizes, m_iup_button_volumesizes, NULL);

  m_iup_text_volumesizes = IupText (NULL);
  IupSetAttribute (m_iup_text_volumesizes, "BORDER", "NO");
  IupSetAttribute (m_iup_text_volumesizes, "READONLY", "YES");
  IupSetAttribute (m_iup_text_volumesizes, "VALUE", "[1.0, 1.0, 1.0]");

  vmethod->SetIupUserInterface (
    IupVbox (
    IupLabel ("Recorte de textura:"),
    m_iup_hbox_slicex,
    m_iup_hbox_slicey,
    m_iup_hbox_slicez,
    m_iup_button_slicesizes,
    m_iup_hbox_volumesizes,
    m_iup_text_volumesizes,
    NULL));
}

void ERN2DUserInterface::SetInitXSlice (int initx)
{
  char s[20];
  sprintf (s, "%d", initx);
  IupSetAttribute (IupGetChild (m_iup_spin_initslicex, 1), "VALUE", s);
}

void ERN2DUserInterface::SetLastXSlice (int lastx)
{
  char s[20];
  sprintf (s, "%d", lastx);
  IupSetAttribute (IupGetChild (m_iup_spin_lastslicex, 1), "VALUE", s);
}

void ERN2DUserInterface::SetInitYSlice (int inity)
{
  char s[20];
  sprintf (s, "%d", inity);
  IupSetAttribute (IupGetChild (m_iup_spin_initslicey, 1), "VALUE", s);
}

void ERN2DUserInterface::SetLastYSlice (int lasty)
{
  char s[20];
  sprintf (s, "%d", lasty);
  IupSetAttribute (IupGetChild (m_iup_spin_lastslicey, 1), "VALUE", s);
}

void ERN2DUserInterface::SetInitZSlice (int initz)
{
  char s[20];
  sprintf (s, "%d", initz);
  IupSetAttribute (IupGetChild (m_iup_spin_initslicez, 1), "VALUE", s);
}

void ERN2DUserInterface::SetLastZSlice (int lastz)
{
  char s[20];
  sprintf (s, "%d", lastz);
  IupSetAttribute (IupGetChild (m_iup_spin_lastslicez, 1), "VALUE", s);
}

void ERN2DUserInterface::UpdateCubeSizes ()
{
  char s[100];
  sprintf (s, "[%.4f, %.4f, %.4f]", m_gui_cubewidth, m_gui_cubeheight, m_gui_cubedepth);
  IupSetAttribute (m_iup_text_volumesizes, "VALUE", s);
}
