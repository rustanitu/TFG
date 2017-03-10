#include "ERNUserInterface.h"

#include "../Viewer.h"
#include "ERNViewMethod.h"

#include <GL/glew.h>
#include <ctime>

#include <glutils/GLShader.h>
#include <glutils/GLUtils.h>
#include <math/MUtils.h>
#include <atfg/ScalarField.h>
#include <atfg/TransferFunction.h>
#include <lqc/lqcdefines.h>

int ERNUserInterface::ListCB_VisModelChange (Ihandle* ih, char *text, int item, int state)
{
  if (state == 1)
  {
    ERNViewMethod* vmethod = (ERNViewMethod*)Viewer::Instance ()->m_viewmethods[VRVIEWS::ERN];
    vmethod->SetRenderMode (item - 1);
  }
  return IUP_DEFAULT;
}

int ERNUserInterface::SpinCB_GLSLSamples (Ihandle* ih, int increment)
{
  ERNViewMethod* vmethod = (ERNViewMethod*)Viewer::Instance ()->m_viewmethods[VRVIEWS::ERN];
  if (increment == 1)
    vmethod->SetSamples (vmethod->GetSamples () * 2);
  else if (increment == -1)
    vmethod->SetSamples (vmethod->GetSamples () / 2);

  char s[20];
  sprintf (s, "%d", vmethod->GetSamples ());
  IupSetAttribute (IupGetChild (ih, 1), "VALUE", s);

  return IUP_DEFAULT;
}

int ERNUserInterface::ToggleCB_GLSLUSEGRADIENT (Ihandle* ih, int state)
{
  ERNViewMethod* vmethod = (ERNViewMethod*)Viewer::Instance ()->m_viewmethods[VRVIEWS::ERN];
  vmethod->SetUseGradient (state);
  return IUP_DEFAULT;
}

int ERNUserInterface::ToggleCB_GLSLITERATE (Ihandle* ih, int state)
{
  ERNViewMethod* vmethod = (ERNViewMethod*)Viewer::Instance ()->m_viewmethods[VRVIEWS::ERN];
  vmethod->SetIterate (state == 1 ? true : false);
  vmethod->SetAutoRedisplay (vmethod->GetIterate ());
  return IUP_DEFAULT;
}

int ERNUserInterface::ListCB_GLSLComposition (Ihandle* ih, char *text, int item, int state)
{
  if (state == 1)
  {
    ERNViewMethod* vmethod = (ERNViewMethod*)Viewer::Instance ()->m_viewmethods[VRVIEWS::ERN];
    vmethod->ResetShaders (text);
  }
  return IUP_DEFAULT;
}

int ERNUserInterface::SpinCB_InitSlicesX (Ihandle* ih, int increment)
{
  ERNViewMethod* vmethod = (ERNViewMethod*)Viewer::Instance ()->m_viewmethods[VRVIEWS::ERN];
  if (increment == 1)
    vmethod->SetInitXSlice (vmethod->GetInitXSlice () + 1);
  else if (increment == -1)
    vmethod->SetInitXSlice (vmethod->GetInitXSlice () - 1);

  return IUP_DEFAULT;
}

int ERNUserInterface::SpinCB_LastSlicesX (Ihandle* ih, int increment)
{
  ERNViewMethod* vmethod = (ERNViewMethod*)Viewer::Instance ()->m_viewmethods[VRVIEWS::ERN];
  if (increment == 1)
    vmethod->SetLastXSlice (vmethod->GetLastXSlice () + 1);
  else if (increment == -1)
    vmethod->SetLastXSlice (vmethod->GetLastXSlice () - 1);

  return IUP_DEFAULT;
}

int ERNUserInterface::SpinCB_InitSlicesY (Ihandle* ih, int increment)
{
  ERNViewMethod* vmethod = (ERNViewMethod*)Viewer::Instance ()->m_viewmethods[VRVIEWS::ERN];
  if (increment == 1)
    vmethod->SetInitYSlice (vmethod->GetInitYSlice () + 1);
  else if (increment == -1)
    vmethod->SetInitYSlice (vmethod->GetInitYSlice () - 1);

  return IUP_DEFAULT;
}

int ERNUserInterface::SpinCB_LastSlicesY (Ihandle* ih, int increment)
{
  ERNViewMethod* vmethod = (ERNViewMethod*)Viewer::Instance ()->m_viewmethods[VRVIEWS::ERN];
  if (increment == 1)
    vmethod->SetLastYSlice (vmethod->GetLastYSlice () + 1);
  else if (increment == -1)
    vmethod->SetLastYSlice (vmethod->GetLastYSlice () - 1);

  return IUP_DEFAULT;
}

int ERNUserInterface::SpinCB_InitSlicesZ (Ihandle* ih, int increment)
{
  ERNViewMethod* vmethod = (ERNViewMethod*)Viewer::Instance ()->m_viewmethods[VRVIEWS::ERN];
  if (increment == 1)
    vmethod->SetInitZSlice (vmethod->GetInitZSlice () + 1);
  else if (increment == -1)
    vmethod->SetInitZSlice (vmethod->GetInitZSlice () - 1);

  return IUP_DEFAULT;
}

int ERNUserInterface::SpinCB_LastSlicesZ (Ihandle* ih, int increment)
{
  ERNViewMethod* vmethod = (ERNViewMethod*)Viewer::Instance ()->m_viewmethods[VRVIEWS::ERN];
  if (increment == 1)
    vmethod->SetLastZSlice (vmethod->GetLastZSlice () + 1);
  else if (increment == -1)
    vmethod->SetLastZSlice (vmethod->GetLastZSlice () - 1);

  return IUP_DEFAULT;
}

int ERNUserInterface::ButtonCB_ReloadTextures (Ihandle* ih)
{
  ERNViewMethod* vmethod = (ERNViewMethod*)Viewer::Instance ()->m_viewmethods[VRVIEWS::ERN];
  vmethod->ApplyInputSliceSizes ();
  return IUP_DEFAULT;
}

int ERNUserInterface::ButtonCB_AutoModeling (Ihandle* ih)
{
  ERNViewMethod* vmethod = (ERNViewMethod*)Viewer::Instance ()->m_viewmethods[VRVIEWS::ERN];
  vmethod->AutoModeling ();
  vmethod->MarkOutdated ();
  return IUP_DEFAULT;
}

ERNUserInterface::ERNUserInterface ()
{
  m_gui_cubewidth = 1.0f;
  m_gui_cubeheight = 1.0f;
  m_gui_cubedepth = 1.0f;
}

ERNUserInterface::~ERNUserInterface ()
{

}

void ERNUserInterface::CreateIupInterface ()
{
  ERNViewMethod* vmethod = (ERNViewMethod*)Viewer::Instance ()->m_viewmethods[VRVIEWS::ERN];

  m_iup_list_vismodes = IupList (NULL);
  IupSetAttribute (m_iup_list_vismodes, "DROPDOWN", "YES");
  IupSetAttribute (m_iup_list_vismodes, "1", "('1') Visualização do modelo");
  IupSetAttribute (m_iup_list_vismodes, "2", "('2') Visualização das texturas");
  IupSetAttribute (m_iup_list_vismodes, "VALUE", "1");
  IupSetCallback (m_iup_list_vismodes, "ACTION", (Icallback)ERNUserInterface::ListCB_VisModelChange);

  m_iup_label_samples = IupLabel ("Amostras ('a' | 's'):");
  m_iup_spinbox_samples = IupSpinbox (IupText (NULL));
  char glsl_s[20];
  sprintf (glsl_s, "%d", vmethod->GetSamples ());
  IupSetAttribute (IupGetChild (m_iup_spinbox_samples, 1), "VALUE", glsl_s);
  IupSetCallback (m_iup_spinbox_samples, "SPIN_CB", (Icallback)SpinCB_GLSLSamples);

  m_iup_hbox_samples = IupHbox (m_iup_label_samples, m_iup_spinbox_samples, NULL);

  m_iup_toggle_usegradient = IupToggle ("Usar gradientes ('g')", NULL);
  IupSetAttribute (m_iup_toggle_usegradient, "VALUE", vmethod->GetUseGradient () == 1 ? "YES" : "NO");
  IupSetCallback (m_iup_toggle_usegradient, "ACTION", (Icallback)ToggleCB_GLSLUSEGRADIENT);

  m_iup_toggle_iterate = IupToggle ("Rotacionar modelo (' ')", NULL);
  IupSetAttribute (m_iup_toggle_iterate, "VALUE", vmethod->GetIterate () ? "YES" : "NO");
  IupSetCallback (m_iup_toggle_iterate, "ACTION", (Icallback)ToggleCB_GLSLITERATE);

  m_iup_label_rendertype = IupLabel ("Modo de Composição (Shader):");
  m_iup_list_rendertype = IupList (NULL);
  IupSetAttribute (m_iup_list_rendertype, "DROPDOWN", "YES");
  IupSetAttribute (m_iup_list_rendertype, "1", "Riemann");
  IupSetAttribute (m_iup_list_rendertype, "2", "SecondOrderAdaptive");
  IupSetAttribute (m_iup_list_rendertype, "VALUE", "1");
  IupSetCallback (m_iup_list_rendertype, "ACTION", (Icallback)ERNUserInterface::ListCB_GLSLComposition);

  m_iup_label_slicesx = IupLabel ("X: ");
  m_iup_spin_initslicex = IupSpinbox (IupText (NULL));
  IupSetCallback (m_iup_spin_initslicex, "SPIN_CB", (Icallback)ERNUserInterface::SpinCB_InitSlicesX);
  m_iup_spin_lastslicex = IupSpinbox (IupText (NULL));
  IupSetCallback (m_iup_spin_lastslicex, "SPIN_CB", (Icallback)ERNUserInterface::SpinCB_LastSlicesX);
  m_iup_hbox_slicex = IupHbox (m_iup_label_slicesx, m_iup_spin_initslicex, m_iup_spin_lastslicex, NULL);

  m_iup_label_slicesy = IupLabel ("Y: ");
  m_iup_spin_initslicey = IupSpinbox (IupText (NULL));
  IupSetCallback (m_iup_spin_initslicey, "SPIN_CB", (Icallback)ERNUserInterface::SpinCB_InitSlicesY);
  m_iup_spin_lastslicey = IupSpinbox (IupText (NULL));
  IupSetCallback (m_iup_spin_lastslicey, "SPIN_CB", (Icallback)ERNUserInterface::SpinCB_LastSlicesY);
  m_iup_hbox_slicey = IupHbox (m_iup_label_slicesy, m_iup_spin_initslicey, m_iup_spin_lastslicey, NULL);

  m_iup_label_slicesz = IupLabel ("Z: ");
  m_iup_spin_initslicez = IupSpinbox (IupText (NULL));
  IupSetCallback (m_iup_spin_initslicez, "SPIN_CB", (Icallback)ERNUserInterface::SpinCB_InitSlicesZ);
  m_iup_spin_lastslicez = IupSpinbox (IupText (NULL));
  IupSetCallback (m_iup_spin_lastslicez, "SPIN_CB", (Icallback)ERNUserInterface::SpinCB_LastSlicesZ);
  m_iup_hbox_slicez = IupHbox (m_iup_label_slicesz, m_iup_spin_initslicez, m_iup_spin_lastslicez, NULL);

  m_iup_button_slicesizes = IupButton ("Aplicar", NULL);
  IupSetAttribute (m_iup_button_slicesizes, "FGCOLOR", "255 0 0");;
  IupSetCallback (m_iup_button_slicesizes, "ACTION", (Icallback)ERNUserInterface::ButtonCB_ReloadTextures);

  m_iup_label_volumesizes = IupLabel ("Geometria:");
  m_iup_button_volumesizes = IupButton ("AutoModelar", NULL);
  IupSetAttribute (m_iup_button_volumesizes, "FGCOLOR", "255 0 0");
  IupSetCallback (m_iup_button_volumesizes, "ACTION", (Icallback)ERNUserInterface::ButtonCB_AutoModeling);
  m_iup_hbox_volumesizes = IupHbox (m_iup_label_volumesizes, m_iup_button_volumesizes, NULL);

  m_iup_text_volumesizes = IupText (NULL);
  IupSetAttribute (m_iup_text_volumesizes, "BORDER", "NO");
  IupSetAttribute (m_iup_text_volumesizes, "READONLY", "YES");
  IupSetAttribute (m_iup_text_volumesizes, "VALUE", "[1.0, 1.0, 1.0]");

  vmethod->SetIupUserInterface (
    IupVbox (
    //m_iup_list_vismodes,
    // m_iup_hbox_samples,
    m_iup_toggle_usegradient,
    m_iup_toggle_iterate,
    //m_iup_label_rendertype,
    //m_iup_list_rendertype,
    IupLabel ("Recorte de textura:"),
    m_iup_hbox_slicex,
    m_iup_hbox_slicey,
    m_iup_hbox_slicez,
    m_iup_button_slicesizes,
    m_iup_hbox_volumesizes,
    m_iup_text_volumesizes,
    NULL));
}

void ERNUserInterface::SetVisualizationModel (int render_mode)
{
  char glsl_vm[20];
  sprintf (glsl_vm, "%d", render_mode + 1);
  IupSetAttribute (m_iup_list_vismodes, "VALUE", glsl_vm);
}

void ERNUserInterface::SetNumberOfSamples (int samples)
{
  char glsl_s[20];
  sprintf (glsl_s, "%d", samples);
  IupSetAttribute (IupGetChild (m_iup_spinbox_samples, 1), "VALUE", glsl_s);
}

void ERNUserInterface::SetGradientToggle (int gtoggle)
{
  IupSetAttribute (m_iup_toggle_usegradient, "VALUE", gtoggle == 1 ? "YES" : "NO");
}

void ERNUserInterface::SetIterateToggle (bool itoggle)
{
  IupSetAttribute (m_iup_toggle_iterate, "VALUE", itoggle ? "YES" : "NO");
}

void ERNUserInterface::SetInitXSlice (int initx)
{
  char s[20];
  sprintf (s, "%d", initx);
  IupSetAttribute (IupGetChild (m_iup_spin_initslicex, 1), "VALUE", s);
}

void ERNUserInterface::SetLastXSlice (int lastx)
{
  char s[20];
  sprintf (s, "%d", lastx);
  IupSetAttribute (IupGetChild (m_iup_spin_lastslicex, 1), "VALUE", s);
}

void ERNUserInterface::SetInitYSlice (int inity)
{
  char s[20];
  sprintf (s, "%d", inity);
  IupSetAttribute (IupGetChild (m_iup_spin_initslicey, 1), "VALUE", s);
}

void ERNUserInterface::SetLastYSlice (int lasty)
{
  char s[20];
  sprintf (s, "%d", lasty);
  IupSetAttribute (IupGetChild (m_iup_spin_lastslicey, 1), "VALUE", s);
}

void ERNUserInterface::SetInitZSlice (int initz)
{
  char s[20];
  sprintf (s, "%d", initz);
  IupSetAttribute (IupGetChild (m_iup_spin_initslicez, 1), "VALUE", s);
}

void ERNUserInterface::SetLastZSlice (int lastz)
{
  char s[20];
  sprintf (s, "%d", lastz);
  IupSetAttribute (IupGetChild (m_iup_spin_lastslicez, 1), "VALUE", s);
}

void ERNUserInterface::UpdateCubeSizes ()
{
  char s[100];
  sprintf (s, "[%.4f, %.4f, %.4f]", m_gui_cubewidth, m_gui_cubeheight, m_gui_cubedepth);
  IupSetAttribute (m_iup_text_volumesizes, "VALUE", s);
}
