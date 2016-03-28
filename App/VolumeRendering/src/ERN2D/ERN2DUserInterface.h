/*!
\file ERN2DUserInterface.h
\brief ERN2D ViewMethod GUI.
\author Leonardo Quatrin Campagnolo
*/

#ifndef ERN2DUSERINTERFACE_H
#define ERN2DUSERINTERFACE_H

#include <iup.h>
#include <iupgl.h>

class ERN2DUserInterface
{
public:
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
  ERN2DUserInterface ();
  /*! Destructor*/
  ~ERN2DUserInterface ();

  /*! Generate all iup elements of the gui.*/
  void CreateIupInterface ();

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