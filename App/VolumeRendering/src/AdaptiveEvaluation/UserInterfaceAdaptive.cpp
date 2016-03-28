#include "UserInterfaceAdaptive.h"
#include "ViewMethodAdaptive.h"

int UserInterfaceAdaptive::ListCB_ShaderNumericalMethodChange (Ihandle* ih, char *text, int item, int state)
{
  ViewMethodAdaptive* vmethod = (ViewMethodAdaptive*)Viewer::Instance ()->m_viewmethods[VRVIEWS::ADAPTIVE_GLSL];
  if (state == 1)
    vmethod->ChangeCurrentShaderProgram(item - 1);
  return IUP_DEFAULT;
}

UserInterfaceAdaptive::UserInterfaceAdaptive ()
{}

UserInterfaceAdaptive::~UserInterfaceAdaptive ()
{}

void UserInterfaceAdaptive::CreateIupInterface ()
{
  ViewMethodAdaptive* vmethod = (ViewMethodAdaptive*)Viewer::Instance ()->m_viewmethods[VRVIEWS::ADAPTIVE_GLSL];
 
  m_iup_lbl_shaders = IupLabel ("Método Adaptativo");
  m_iup_lst_shaders = IupList (NULL);
  IupSetAttribute (m_iup_lst_shaders, "1", "Acoplada (Coupled)");
  IupSetAttribute (m_iup_lst_shaders, "2", "Desacoplada (Decoupled)");

  IupSetAttribute (m_iup_lst_shaders, "VALUE", "1");
  IupSetCallback (m_iup_lst_shaders, "ACTION", (Icallback)UserInterfaceAdaptive::ListCB_ShaderNumericalMethodChange);
  IupSetAttribute (m_iup_lst_shaders, "EXPAND", "HORIZONTAL");

  m_vbox = IupVbox (
    m_iup_lbl_shaders,
    m_iup_lst_shaders,
    NULL);

  vmethod->SetIupUserInterface (m_vbox);
}