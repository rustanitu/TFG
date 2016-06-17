/*!
\file IASUserInterface.h
\brief IAS ViewMethod GUI.
\author Leonardo Quatrin Campagnolo
*/

#ifndef ADAPTIVE_GLSL_USERINTERFACE_H
#define ADAPTIVE_GLSL_USERINTERFACE_H

#include <iup.h>
#include <iupgl.h>

class UserInterfaceAdaptive
{
public:
  static int ListCB_ShaderNumericalMethodChange (Ihandle* ih, char *text, int item, int state);

public:
  /*! Constructor*/
  UserInterfaceAdaptive ();
  /*! Destructor*/
  ~UserInterfaceAdaptive ();

  /*! Generate all iup elements of the gui.*/
  void CreateIupInterface ();

private:
  Ihandle* m_vbox;

  Ihandle* m_iup_lbl_shaders;
  Ihandle* m_iup_lst_shaders;
};

#endif