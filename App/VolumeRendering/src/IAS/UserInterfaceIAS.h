/*!
\file IASUserInterface.h
\brief IAS ViewMethod GUI.
\author Leonardo Quatrin Campagnolo
*/

#ifndef IASUSERINTERFACE_H
#define IASUSERINTERFACE_H

#include <iup.h>
#include <iupgl.h>

class UserInterfaceIAS
{
public:
  static int CB_List_ChangeShader (Ihandle* ih, char *text, int item, int state);

  /*! Iup callback when the number of samples is changed by the spinbox.
  */
  static int Callback_tgl_UseDoublePrecision (Ihandle* ih, int state);
  /*! Iup callback to automatically rotate the volume.
  */

  static int ListCB_GLSLComposition (Ihandle* ih, char *text, int item, int state);

  static int SpinCB_InitSlicesX (Ihandle* ih, int increment);
  static int SpinCB_LastSlicesX (Ihandle* ih, int increment);
  static int SpinCB_InitSlicesY (Ihandle* ih, int increment);
  static int SpinCB_LastSlicesY (Ihandle* ih, int increment);
  static int SpinCB_InitSlicesZ (Ihandle* ih, int increment);
  static int SpinCB_LastSlicesZ (Ihandle* ih, int increment);

  static int ButtonCB_ReloadTextures (Ihandle* ih);

  static int ButtonCB_AutoModeling (Ihandle* ih);

public:
  /*! Constructor*/
  UserInterfaceIAS ();
  /*! Destructor*/
  ~UserInterfaceIAS ();

  /*! Generate all iup elements of the gui.*/
  void CreateIupInterface ();

  /*! Set the value of the iup_toggle_usegradient.*/
  void SetGradientToggle (int gtoggle);

  void SetInitXSlice (int initx);
  void SetLastXSlice (int lastx);
  void SetInitYSlice (int inity);
  void SetLastYSlice (int lasty);
  void SetInitZSlice (int initz);
  void SetLastZSlice (int lastz);

  void UpdateCubeSizes ();

  float m_gui_cubewidth;
  float m_gui_cubeheight;
  float m_gui_cubedepth;

private:

  Ihandle* m_iup_text_cubesizes;

  /*! Toggle to control the use of the gradient texture.*/
  Ihandle* m_iup_label_shaders;
  Ihandle* m_iup_list_shaders;
  
  Ihandle* m_iup_tgl_usedoubleprecision;

  Ihandle* m_iup_label_slicesx;
  Ihandle* m_iup_spin_initslicex;
  Ihandle* m_iup_spin_lastslicex;
  Ihandle* m_iup_hbox_slicex;

  Ihandle* m_iup_label_slicesy;
  Ihandle* m_iup_spin_initslicey;
  Ihandle* m_iup_spin_lastslicey;
  Ihandle* m_iup_hbox_slicey;

  Ihandle* m_iup_label_slicesz;
  Ihandle* m_iup_spin_initslicez;
  Ihandle* m_iup_spin_lastslicez;
  Ihandle* m_iup_hbox_slicez;

  Ihandle* m_iup_button_slicesizes;

  Ihandle* m_iup_label_volumesizes;
  Ihandle* m_iup_text_volumesizes;
  Ihandle* m_iup_button_volumesizes;
  Ihandle* m_iup_hbox_volumesizes;
};


#endif