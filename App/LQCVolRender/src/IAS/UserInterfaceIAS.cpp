#include "UserInterfaceIAS.h"

#include "../Viewer.h"
#include "ViewMethodIAS.h"

#include <GL/glew.h>
#include <ctime>

#include <glutils/GLShader.h>
#include <glutils/GLUtils.h>
#include <math/MUtils.h>
#include <atfg/Volume.h>
#include <atfg/TransferFunction.h>
#include <lqc/lqcdefines.h>

int UserInterfaceIAS::CB_List_ChangeShader (Ihandle* ih, char *text, int item, int state)
{
  if (state == 1)
  {
    ViewMethodIAS* vmethod = (ViewMethodIAS*)Viewer::Instance ()->m_viewmethods[VRVIEWS::IAS];
    vmethod->ChangeCurrentShaderProgram (item);
  }
  return IUP_DEFAULT;
}


int UserInterfaceIAS::Callback_tgl_UseDoublePrecision (Ihandle* ih, int state)
{
  ViewMethodIAS* vmethod = (ViewMethodIAS*)Viewer::Instance ()->m_viewmethods[VRVIEWS::IAS];
  vmethod->UseDoublePrecision (state == 1 ? true : false);
  return IUP_DEFAULT;
}

int UserInterfaceIAS::ListCB_GLSLComposition (Ihandle* ih, char *text, int item, int state)
{
  if (state == 1)
  {
    ViewMethodIAS* vmethod = (ViewMethodIAS*)Viewer::Instance ()->m_viewmethods[VRVIEWS::IAS];
    vmethod->ResetShaders (text);
  }
  return IUP_DEFAULT;
}

int UserInterfaceIAS::SpinCB_InitSlicesX (Ihandle* ih, int increment)
{
  ViewMethodIAS* vmethod = (ViewMethodIAS*)Viewer::Instance ()->m_viewmethods[VRVIEWS::IAS];
  if (increment == 1)
    vmethod->SetInitXSlice (vmethod->GetInitXSlice () + 1);
  else if (increment == -1)
    vmethod->SetInitXSlice (vmethod->GetInitXSlice () - 1);

  return IUP_DEFAULT;
}

int UserInterfaceIAS::SpinCB_LastSlicesX (Ihandle* ih, int increment)
{
  ViewMethodIAS* vmethod = (ViewMethodIAS*)Viewer::Instance ()->m_viewmethods[VRVIEWS::IAS];
  if (increment == 1)
    vmethod->SetLastXSlice (vmethod->GetLastXSlice () + 1);
  else if (increment == -1)
    vmethod->SetLastXSlice (vmethod->GetLastXSlice () - 1);

  return IUP_DEFAULT;
}

int UserInterfaceIAS::SpinCB_InitSlicesY (Ihandle* ih, int increment)
{
  ViewMethodIAS* vmethod = (ViewMethodIAS*)Viewer::Instance ()->m_viewmethods[VRVIEWS::IAS];
  if (increment == 1)
    vmethod->SetInitYSlice (vmethod->GetInitYSlice () + 1);
  else if (increment == -1)
    vmethod->SetInitYSlice (vmethod->GetInitYSlice () - 1);

  return IUP_DEFAULT;
}

int UserInterfaceIAS::SpinCB_LastSlicesY (Ihandle* ih, int increment)
{
  ViewMethodIAS* vmethod = (ViewMethodIAS*)Viewer::Instance ()->m_viewmethods[VRVIEWS::IAS];
  if (increment == 1)
    vmethod->SetLastYSlice (vmethod->GetLastYSlice () + 1);
  else if (increment == -1)
    vmethod->SetLastYSlice (vmethod->GetLastYSlice () - 1);

  return IUP_DEFAULT;
}

int UserInterfaceIAS::SpinCB_InitSlicesZ (Ihandle* ih, int increment)
{
  ViewMethodIAS* vmethod = (ViewMethodIAS*)Viewer::Instance ()->m_viewmethods[VRVIEWS::IAS];
  if (increment == 1)
    vmethod->SetInitZSlice (vmethod->GetInitZSlice () + 1);
  else if (increment == -1)
    vmethod->SetInitZSlice (vmethod->GetInitZSlice () - 1);

  return IUP_DEFAULT;
}

int UserInterfaceIAS::SpinCB_LastSlicesZ (Ihandle* ih, int increment)
{
  ViewMethodIAS* vmethod = (ViewMethodIAS*)Viewer::Instance ()->m_viewmethods[VRVIEWS::IAS];
  if (increment == 1)
    vmethod->SetLastZSlice (vmethod->GetLastZSlice () + 1);
  else if (increment == -1)
    vmethod->SetLastZSlice (vmethod->GetLastZSlice () - 1);

  return IUP_DEFAULT;
}

int UserInterfaceIAS::ButtonCB_ReloadTextures (Ihandle* ih)
{
  ViewMethodIAS* vmethod = (ViewMethodIAS*)Viewer::Instance ()->m_viewmethods[VRVIEWS::IAS];
  vmethod->ApplyInputSliceSizes ();
  return IUP_DEFAULT;
}

int UserInterfaceIAS::ButtonCB_AutoModeling (Ihandle* ih)
{
  ViewMethodIAS* vmethod = (ViewMethodIAS*)Viewer::Instance ()->m_viewmethods[VRVIEWS::IAS];
  vmethod->AutoModeling ();
  vmethod->MarkOutdated ();
  return IUP_DEFAULT;
}

UserInterfaceIAS::UserInterfaceIAS ()
{
  m_gui_cubewidth = 1.0f;
  m_gui_cubeheight = 1.0f;
  m_gui_cubedepth = 1.0f;
}

UserInterfaceIAS::~UserInterfaceIAS ()
{

}

void UserInterfaceIAS::CreateIupInterface ()
{
  ViewMethodIAS* vmethod = (ViewMethodIAS*)Viewer::Instance ()->m_viewmethods[VRVIEWS::IAS];

  m_iup_label_shaders = IupLabel ("Shaders:");
  m_iup_list_shaders = IupList (NULL);
  //Riemman Summation
  //IupSetAttribute (m_iup_list_rendermodes, "DROPDOWN", "YES");
  //IupSetAttribute (m_iup_list_shaders, "1", "Simpson Half Half");
  //IupSetAttribute (m_iup_list_shaders, "2", "Simpson Half Queue");
  //IupSetAttribute (m_iup_list_shaders, "3", "Simpson Half Queue Acc");
  //IupSetAttribute (m_iup_list_shaders, "4", "Simpson Half Iteration");
  //IupSetAttribute (m_iup_list_shaders, "5", "Simpson Half Iter opt");
  //IupSetAttribute (m_iup_list_shaders, "6", "Simpson Half Iter opt2");
  IupSetAttribute (m_iup_list_shaders, "1", "(A) Coupled");
  //IupSetAttribute (m_iup_list_shaders, "8", "Simple Adaptive");
  //IupSetAttribute (m_iup_list_shaders, "9", "Simpson HI S ExtStep");
  //IupSetAttribute (m_iup_list_shaders, "10", "Simpson HI C ExtStep");
  //IupSetAttribute (m_iup_list_shaders, "11", "Separated Adaptive");
  IupSetAttribute (m_iup_list_shaders, "2", "(A) Decoupled");
  //IupSetAttribute (m_iup_list_shaders, "13", "Opt Simpson HI S ES");
  //IupSetAttribute (m_iup_list_shaders, "14", "Simpson Half Iterate exp");
  //IupSetAttribute (m_iup_list_shaders, "2", "(F) Simpson Error");
  //IupSetAttribute (m_iup_list_shaders, "3", "(F) Simpson Error Projection");
  //IupSetAttribute (m_iup_list_shaders, "4", "(F) Simpson Error Half Proj");
  //IupSetAttribute (m_iup_list_shaders, "5", "(F) Simpson Error Half");
  //IupSetAttribute (m_iup_list_shaders, "7", "(D) Simpson Half");
  IupSetAttribute (m_iup_list_shaders, "VALUE", "1");
  IupSetCallback (m_iup_list_shaders, "ACTION", (Icallback)UserInterfaceIAS::CB_List_ChangeShader);
  IupSetAttribute (m_iup_list_shaders, "EXPAND", "HORIZONTAL");

  m_iup_tgl_usedoubleprecision = IupToggle ("Usar dupla precisão", NULL);
  IupSetCallback (m_iup_tgl_usedoubleprecision, "ACTION", (Icallback)Callback_tgl_UseDoublePrecision);

  m_iup_label_slicesx = IupLabel ("X: ");
  m_iup_spin_initslicex = IupSpinbox (IupText (NULL));
  IupSetCallback (m_iup_spin_initslicex, "SPIN_CB", (Icallback)UserInterfaceIAS::SpinCB_InitSlicesX);
  m_iup_spin_lastslicex = IupSpinbox (IupText (NULL));
  IupSetCallback (m_iup_spin_lastslicex, "SPIN_CB", (Icallback)UserInterfaceIAS::SpinCB_LastSlicesX);
  m_iup_hbox_slicex = IupHbox (m_iup_label_slicesx, m_iup_spin_initslicex, m_iup_spin_lastslicex, NULL);

  m_iup_label_slicesy = IupLabel ("Y: ");
  m_iup_spin_initslicey = IupSpinbox (IupText (NULL));
  IupSetCallback (m_iup_spin_initslicey, "SPIN_CB", (Icallback)UserInterfaceIAS::SpinCB_InitSlicesY);
  m_iup_spin_lastslicey = IupSpinbox (IupText (NULL));
  IupSetCallback (m_iup_spin_lastslicey, "SPIN_CB", (Icallback)UserInterfaceIAS::SpinCB_LastSlicesY);
  m_iup_hbox_slicey = IupHbox (m_iup_label_slicesy, m_iup_spin_initslicey, m_iup_spin_lastslicey, NULL);

  m_iup_label_slicesz = IupLabel ("Z: ");
  m_iup_spin_initslicez = IupSpinbox (IupText (NULL));
  IupSetCallback (m_iup_spin_initslicez, "SPIN_CB", (Icallback)UserInterfaceIAS::SpinCB_InitSlicesZ);
  m_iup_spin_lastslicez = IupSpinbox (IupText (NULL));
  IupSetCallback (m_iup_spin_lastslicez, "SPIN_CB", (Icallback)UserInterfaceIAS::SpinCB_LastSlicesZ);
  m_iup_hbox_slicez = IupHbox (m_iup_label_slicesz, m_iup_spin_initslicez, m_iup_spin_lastslicez, NULL);

  m_iup_button_slicesizes = IupButton ("Aplicar", NULL);
  IupSetCallback (m_iup_button_slicesizes, "ACTION", (Icallback)UserInterfaceIAS::ButtonCB_ReloadTextures);

  m_iup_label_volumesizes = IupLabel ("Geometria:");
  m_iup_button_volumesizes = IupButton ("Redimensionar Geometria", NULL);
  IupSetCallback (m_iup_button_volumesizes, "ACTION", (Icallback)UserInterfaceIAS::ButtonCB_AutoModeling);
  //m_iup_hbox_volumesizes = IupHbox (m_iup_label_volumesizes, m_iup_button_volumesizes, NULL);
  
  m_iup_text_volumesizes = IupText (NULL);
  IupSetAttribute (m_iup_text_volumesizes, "BORDER", "NO");
  IupSetAttribute (m_iup_text_volumesizes, "READONLY", "YES");
  IupSetAttribute (m_iup_text_volumesizes, "VALUE", "[1.0, 1.0, 1.0]");

  vmethod->SetIupUserInterface (
    IupVbox (
    m_iup_label_shaders,
    m_iup_list_shaders,
    //m_iup_tgl_usedoubleprecision,
    m_iup_button_volumesizes,
    IupLabel ("Recorte:"),
    m_iup_hbox_slicex, 
    m_iup_hbox_slicey, 
    m_iup_hbox_slicez, 
    m_iup_button_slicesizes,
    //m_iup_hbox_volumesizes,
    //m_iup_text_volumesizes,
    NULL));
}

void UserInterfaceIAS::SetGradientToggle (int gtoggle)
{
  //IupSetAttribute (m_iup_toggle_usegradient, "VALUE", gtoggle == 1 ? "YES" : "NO");
}


void UserInterfaceIAS::SetInitXSlice (int initx)
{
  char s[20];
  sprintf (s, "%d", initx);
  IupSetAttribute (IupGetChild (m_iup_spin_initslicex, 1), "VALUE", s);
}

void UserInterfaceIAS::SetLastXSlice (int lastx)
{
  char s[20];
  sprintf (s, "%d", lastx);
  IupSetAttribute (IupGetChild (m_iup_spin_lastslicex, 1), "VALUE", s);
}

void UserInterfaceIAS::SetInitYSlice (int inity)
{
  char s[20];
  sprintf (s, "%d", inity);
  IupSetAttribute (IupGetChild (m_iup_spin_initslicey, 1), "VALUE", s);
}

void UserInterfaceIAS::SetLastYSlice (int lasty)
{
  char s[20];
  sprintf (s, "%d", lasty);
  IupSetAttribute (IupGetChild (m_iup_spin_lastslicey, 1), "VALUE", s);
}

void UserInterfaceIAS::SetInitZSlice (int initz)
{
  char s[20];
  sprintf (s, "%d", initz);
  IupSetAttribute (IupGetChild (m_iup_spin_initslicez, 1), "VALUE", s);
}

void UserInterfaceIAS::SetLastZSlice (int lastz)
{
  char s[20];
  sprintf (s, "%d", lastz);
  IupSetAttribute (IupGetChild (m_iup_spin_lastslicez, 1), "VALUE", s);
}

void UserInterfaceIAS::UpdateCubeSizes ()
{
  char s[100];
  sprintf (s, "[%.4f, %.4f, %.4f]", m_gui_cubewidth, m_gui_cubeheight, m_gui_cubedepth);
  IupSetAttribute (m_iup_text_volumesizes, "VALUE", s);
}
