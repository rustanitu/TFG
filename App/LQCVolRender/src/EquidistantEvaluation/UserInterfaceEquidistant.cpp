#include "UserInterfaceEquidistant.h"
#include "ViewMethodEquidistant.h"

int UserInterfaceEquidistant::ListCB_ShaderNumericalMethodChange (Ihandle* ih, char *text, int item, int state)
{
  ViewMethodEquidistant* vmethod = (ViewMethodEquidistant*)Viewer::Instance()->m_viewmethods[VRVIEWS::EQUIDISTANT_GLSL];
  if (state == 1)
    printf ("Change Shader: %d\n", item - 1);
  //  vmethod->SetCurrentMethod (item - 1);
  return IUP_DEFAULT;
}

UserInterfaceEquidistant::UserInterfaceEquidistant ()
{}

UserInterfaceEquidistant::~UserInterfaceEquidistant ()
{}

void UserInterfaceEquidistant::CreateIupInterface ()
{
  ViewMethodEquidistant* vmethod = (ViewMethodEquidistant*)Viewer::Instance()->m_viewmethods[VRVIEWS::EQUIDISTANT_GLSL];
  
  m_iup_lbl_shaders = IupLabel ("Método numérico");
  m_iup_lst_shaders = IupList (NULL);
  IupSetAttribute (m_iup_lst_shaders, "1", "Soma de Riemann");
  //IupSetAttribute (m_iup_lst_shaders, "2", "Trapezoidal Rule");
  //IupSetAttribute (m_iup_lst_shaders, "3", "Simpson Rule");

  IupSetAttribute (m_iup_lst_shaders, "VALUE", "1");
  IupSetCallback (m_iup_lst_shaders, "ACTION", (Icallback)UserInterfaceEquidistant::ListCB_ShaderNumericalMethodChange);
  IupSetAttribute (m_iup_lst_shaders, "EXPAND", "HORIZONTAL");

  m_vbox = IupVbox (
    m_iup_lbl_shaders,
    m_iup_lst_shaders,
    NULL);

  vmethod->SetIupUserInterface (m_vbox);
}