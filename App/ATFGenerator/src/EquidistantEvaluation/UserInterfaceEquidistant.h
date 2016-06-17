/*!
\file RGLSLUserInterface.h
\brief RGLSL ViewMethod GUI.
\author Leonardo Quatrin Campagnolo
*/

#ifndef RGLSL_USERINTERFACE_H
#define RGLSL_USERINTERFACE_H

#include <iup.h>
#include <iupgl.h>

class UserInterfaceEquidistant
{
public:
  static int ListCB_ShaderNumericalMethodChange (Ihandle* ih, char *text, int item, int state);

public:
  /*! Constructor*/
  UserInterfaceEquidistant ();
  /*! Destructor*/
  ~UserInterfaceEquidistant ();

  /*! Generate all iup elements of the gui.*/
  void CreateIupInterface ();

private:
  Ihandle* m_vbox;

  Ihandle* m_iup_lbl_shaders;
  Ihandle* m_iup_lst_shaders;
};

#endif